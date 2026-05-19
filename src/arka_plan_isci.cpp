#include "arka_plan_isci.h"

#include <QFile>
#include <QMetaObject>
#include <QStringList>
#include <QTextStream>
#include <QThread>

#include <algorithm>
#include <chrono>
#include <thread>

ArkaPlanIsci::ArkaPlanIsci(QObject* parent)
    : QObject(parent)
{
}

void ArkaPlanIsci::iptalEt() {
    iptalIstendi.store(true);
}

bool ArkaPlanIsci::satirdanKitapOlustur(const QString& satir, Kitap& kitap) const {
    QString temizSatir = satir.trimmed();

    if (temizSatir.isEmpty()) {
        return false;
    }

    if (temizSatir.toLower().startsWith("isbn")) {
        return false;
    }

    QStringList alanlar = temizSatir.split(';');

    if (alanlar.size() < 6) {
        alanlar = temizSatir.split(',');
    }

    if (alanlar.size() < 6) {
        return false;
    }

    bool yilTamam = false;
    bool kopyaTamam = false;

    const int yayinYili = alanlar[3].trimmed().toInt(&yilTamam);
    const int kopyaSayisi = alanlar[5].trimmed().toInt(&kopyaTamam);

    if (!yilTamam || !kopyaTamam) {
        return false;
    }

    kitap = {
        alanlar[0].trimmed().toStdString(),
        alanlar[1].trimmed().toStdString(),
        alanlar[2].trimmed().toStdString(),
        yayinYili,
        alanlar[4].trimmed().toStdString(),
        kopyaSayisi
    };

    return true;
}

void ArkaPlanIsci::csvKatalogYukle(const QString& dosyaYolu) {
    iptalIstendi.store(false);

    QFile sayimDosyasi(dosyaYolu);

    if (!sayimDosyasi.open(QIODevice::ReadOnly | QIODevice::Text)) {
        emit hataOlustu("CSV dosyasi acilamadi.");
        return;
    }

    int toplamSatir = 0;
    QTextStream sayimAkimi(&sayimDosyasi);

    while (!sayimAkimi.atEnd()) {
        const QString satir = sayimAkimi.readLine().trimmed();

        if (!satir.isEmpty() && !satir.toLower().startsWith("isbn")) {
            ++toplamSatir;
        }
    }

    sayimDosyasi.close();

    if (toplamSatir == 0) {
        emit hataOlustu("CSV dosyasinda okunabilir kitap kaydi bulunamadi.");
        return;
    }

    QFile dosya(dosyaYolu);

    if (!dosya.open(QIODevice::ReadOnly | QIODevice::Text)) {
        emit hataOlustu("CSV dosyasi tekrar acilamadi.");
        return;
    }

    QTextStream akim(&dosya);

    KitapListesi yuklenenKitaplar;
    int okunanSatir = 0;

    while (!akim.atEnd()) {
        if (iptalIstendi.load()) {
            dosya.close();
            emit iptalEdildi();
            return;
        }

        const QString satir = akim.readLine();

        Kitap kitap;
        if (satirdanKitapOlustur(satir, kitap)) {
            {
                std::lock_guard<std::mutex> kilit(veriMutex);
                yuklenenKitaplar.push_back(kitap);
            }

            ++okunanSatir;

            const int yuzde = std::clamp((okunanSatir * 100) / toplamSatir, 0, 100);
            emit ilerlemeGuncellendi(yuzde);
        }

        QThread::msleep(20);
    }

    dosya.close();

    emit ilerlemeGuncellendi(100);
    emit katalogYuklemeTamamlandi(yuklenenKitaplar);
}

void ArkaPlanIsci::gecikmeRaporuOlustur(OduncListesi oduncler) {
    iptalIstendi.store(false);

    const int toplam = static_cast<int>(oduncler.size());

    if (toplam == 0) {
        emit raporTamamlandi("Gecikme raporu olusturuldu.\nKayit bulunamadi.");
        return;
    }

    std::vector<QString> raporSatirlari;

    // std::thread icinden dogrudan emit yapmak Qt thread modeliyle uyumsuz oldugundan
    // QMetaObject::invokeMethod ile Qt::QueuedConnection kullanilarak sinyal
    // guvenli sekilde ana event loop'a iletilmektedir.
    std::thread raporIsParcasi([this, &oduncler, &raporSatirlari, toplam]() {
        for (int i = 0; i < toplam; ++i) {
            if (iptalIstendi.load()) {
                QMetaObject::invokeMethod(this, "iptalEdildi", Qt::QueuedConnection);
                return;
            }

            const auto& kayit = oduncler.at(static_cast<std::size_t>(i));

            if (kayit.durum == OduncDurum::Gecikmis) {
                QString satir = QString("Kayit #%1 | Uye: %2 | ISBN: %3 | Odunc Tarihi: %4")
                    .arg(kayit.kayit_id)
                    .arg(kayit.uye_no)
                    .arg(QString::fromStdString(kayit.isbn))
                    .arg(QString::fromStdString(kayit.odunc_tarihi));

                {
                    std::lock_guard<std::mutex> kilit(veriMutex);
                    raporSatirlari.push_back(satir);
                }
            }

            const int yuzde = std::clamp(((i + 1) * 100) / toplam, 0, 100);
            QMetaObject::invokeMethod(
                this,
                "ilerlemeGuncellendi",
                Qt::QueuedConnection,
                Q_ARG(int, yuzde)
            );

            std::this_thread::sleep_for(std::chrono::milliseconds(80));
        }
    });

    if (raporIsParcasi.joinable()) {
        raporIsParcasi.join();
    }

    if (iptalIstendi.load()) {
        return;
    }

    QString rapor;
    rapor += "KUBYS Gecikme Raporu\n";
    rapor += "====================\n\n";

    if (raporSatirlari.empty()) {
        rapor += "Gecikmis odunc kaydi bulunamadi.\n";
    } else {
        std::lock_guard<std::mutex> kilit(veriMutex);
        for (const auto& satir : raporSatirlari) {
            rapor += satir + "\n";
        }
    }

    emit ilerlemeGuncellendi(100);
    emit raporTamamlandi(rapor);
}