#include "kutuphane_sunucu.h"

#include <QDate>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QHostAddress>

#include <algorithm>
#include <ranges>

KutuphaneSunucu::KutuphaneSunucu(QObject* parent)
    : QObject(parent),
      sunucu(new QTcpServer(this))
{
    connect(sunucu, &QTcpServer::newConnection, this, &KutuphaneSunucu::yeniBaglantiGeldi);
    ornekVerileriOlustur();
}

bool KutuphaneSunucu::baslat(quint16 port) {
    if (sunucu->isListening()) {
        emit durumMesaji("Sunucu zaten calisiyor.");
        return true;
    }

    const bool basarili = sunucu->listen(QHostAddress::Any, port);

    if (basarili) {
        emit durumMesaji(QString("Sunucu baslatildi. Port: %1").arg(port));
    } else {
        emit durumMesaji("Sunucu baslatilamadi: " + sunucu->errorString());
    }

    return basarili;
}

void KutuphaneSunucu::durdur() {
    for (QTcpSocket* istemci : istemciler) {
        if (istemci) {
            istemci->disconnectFromHost();
            istemci->deleteLater();
        }
    }

    istemciler.clear();

    if (sunucu->isListening()) {
        sunucu->close();
    }

    emit istemciSayisiDegisti(0);
    emit durumMesaji("Sunucu durduruldu.");
}

bool KutuphaneSunucu::calisiyorMu() const {
    return sunucu->isListening();
}

void KutuphaneSunucu::ornekVerileriOlustur() {
    std::lock_guard<std::mutex> kilit(veriMutex);

    kitaplar.ekle("978-975-1000-01-1", {
        "978-975-1000-01-1",
        "Modern C++ Temelleri",
        "Selin Arslan",
        2020,
        "Programlama",
        4
    });

    kitaplar.ekle("978-975-1000-02-8", {
        "978-975-1000-02-8",
        "Algoritma Tasarimi",
        "Emre Koc",
        2019,
        "Bilgisayar Bilimi",
        2
    });

    kitaplar.ekle("978-975-1000-03-5", {
        "978-975-1000-03-5",
        "Nesne Yonelimli Programlama",
        "Derya Yalcin",
        2023,
        "Programlama",
        3
    });

    uyeler.ekle(2001, {2001, "Mustafa Emre", "Gulum", "555-1201", "2025-02-03"});
    uyeler.ekle(2002, {2002, "Ali", "Bozkurt", "555-1202", "2025-02-10"});
    uyeler.ekle(2003, {2003, "Arda", "Arslan", "555-1203", "2025-02-18"});

    oduncler.ekle(1, {
        1,
        2001,
        "978-975-1000-01-1",
        "2025-03-01",
        std::nullopt,
        OduncDurum::Oduncte
    });

    oduncler.ekle(2, {
        2,
        2002,
        "978-975-1000-02-8",
        "2025-02-12",
        "2025-02-25",
        OduncDurum::IadeEdildi
    });

    oduncler.ekle(3, {
        3,
        2001,
        "978-975-1000-03-5",
        "2025-01-15",
        std::nullopt,
        OduncDurum::Gecikmis
    });

    sonrakiKayitId = 4;
}

void KutuphaneSunucu::yeniBaglantiGeldi() {
    while (sunucu->hasPendingConnections()) {
        QTcpSocket* istemci = sunucu->nextPendingConnection();

        istemciler.append(istemci);

        connect(istemci, &QTcpSocket::readyRead, this, &KutuphaneSunucu::istemcidenVeriGeldi);
        connect(istemci, &QTcpSocket::disconnected, this, &KutuphaneSunucu::istemciAyrildi);

        emit istemciSayisiDegisti(istemciler.size());
        emit durumMesaji("Yeni istemci baglandi.");

        QJsonObject cevap;
        cevap["durum"] = "bilgi";
        cevap["mesaj"] = "KUBYS sunucusuna baglandiniz.";
        cevapGonder(istemci, cevap);
    }
}

void KutuphaneSunucu::istemcidenVeriGeldi() {
    QTcpSocket* istemci = qobject_cast<QTcpSocket*>(sender());

    if (!istemci) {
        return;
    }

    while (istemci->canReadLine()) {
        const QByteArray satir = istemci->readLine().trimmed();

        if (satir.isEmpty()) {
            continue;
        }

        QJsonParseError hata;
        const QJsonDocument belge = QJsonDocument::fromJson(satir, &hata);

        if (hata.error != QJsonParseError::NoError || !belge.isObject()) {
            QJsonObject cevap;
            cevap["durum"] = "hata";
            cevap["mesaj"] = "Gecersiz JSON istegi.";
            cevapGonder(istemci, cevap);
            continue;
        }

        istekIsle(istemci, belge.object());
    }
}

void KutuphaneSunucu::istemciAyrildi() {
    QTcpSocket* istemci = qobject_cast<QTcpSocket*>(sender());

    if (!istemci) {
        return;
    }

    istemciler.removeAll(istemci);
    istemci->deleteLater();

    emit istemciSayisiDegisti(istemciler.size());
    emit durumMesaji("Bir istemci ayrildi.");
}

void KutuphaneSunucu::istekIsle(QTcpSocket* istemci, const QJsonObject& istek) {
    const QString tip = istek["tip"].toString();

    QJsonObject cevap;

    if (tip == "kimlik_dogrula") {
        cevap = kimlikDogrula(istek);
    } else if (tip == "kitap_ara") {
        cevap = kitapAra(istek);
    } else if (tip == "odunc_al") {
        cevap = oduncAl(istek);
    } else if (tip == "iade_et") {
        cevap = iadeEt(istek);
    } else {
        cevap["durum"] = "hata";
        cevap["mesaj"] = "Bilinmeyen istek tipi.";
    }

    cevapGonder(istemci, cevap);
}

void KutuphaneSunucu::cevapGonder(QTcpSocket* istemci, const QJsonObject& cevap) {
    if (!istemci || istemci->state() != QAbstractSocket::ConnectedState) {
        return;
    }

    const QJsonDocument belge(cevap);
    istemci->write(belge.toJson(QJsonDocument::Compact));
    istemci->write("\n");
    istemci->flush();
}

QJsonObject KutuphaneSunucu::kimlikDogrula(const QJsonObject& istek) {
    const QString kullanici = istek["kullanici"].toString();
    const QString sifre = istek["sifre"].toString();

    QJsonObject cevap;

    if (kullanici == "admin" && sifre == "1234") {
        cevap["durum"] = "basarili";
        cevap["mesaj"] = "Kimlik dogrulandi.";
    } else {
        cevap["durum"] = "hata";
        cevap["mesaj"] = "Kullanici adi veya sifre hatali.";
    }

    return cevap;
}

QJsonObject KutuphaneSunucu::kitapAra(const QJsonObject& istek) {
    const QString aranan = istek["aranan"].toString().trimmed().toLower();

    QJsonArray kitapDizisi;

    {
        std::lock_guard<std::mutex> kilit(veriMutex);

        for (const auto& kitap : kitaplar.tumunu_al() | std::views::values) {
            const QString isbn = QString::fromStdString(kitap.isbn).toLower();
            const QString baslik = QString::fromStdString(kitap.baslik).toLower();
            const QString yazar = QString::fromStdString(kitap.yazar).toLower();
            const QString kategori = QString::fromStdString(kitap.kategori).toLower();

            const bool eslesti =
                aranan.isEmpty() ||
                isbn.contains(aranan) ||
                baslik.contains(aranan) ||
                yazar.contains(aranan) ||
                kategori.contains(aranan);

            if (eslesti) {
                kitapDizisi.append(kitapJson(kitap));
            }
        }
    }

    QJsonObject cevap;
    cevap["durum"] = "basarili";
    cevap["mesaj"] = "Kitap arama tamamlandi.";
    cevap["kitaplar"] = kitapDizisi;

    return cevap;
}

QJsonObject KutuphaneSunucu::oduncAl(const QJsonObject& istek) {
    const int uyeNo = istek["uye_no"].toInt();
    const QString isbnQString = istek["isbn"].toString().trimmed();
    const std::string isbn = isbnQString.toStdString();

    std::lock_guard<std::mutex> kilit(veriMutex);

    QJsonObject cevap;

    if (!uyeler.bul(uyeNo).has_value()) {
        cevap["durum"] = "hata";
        cevap["mesaj"] = "Uye bulunamadi.";
        return cevap;
    }

    auto kitapOpt = kitaplar.bul(isbn);

    if (!kitapOpt.has_value()) {
        cevap["durum"] = "hata";
        cevap["mesaj"] = "Kitap bulunamadi.";
        return cevap;
    }

    Kitap kitap = kitapOpt.value();

    if (kitap.kopya_sayisi <= 0) {
        cevap["durum"] = "hata";
        cevap["mesaj"] = "Kitabin uygun kopyasi yok.";
        return cevap;
    }

    for (const auto& kayit : oduncler.tumunu_al() | std::views::values) {
        if (kayit.uye_no == uyeNo &&
            kayit.isbn == isbn &&
            kayit.durum == OduncDurum::Oduncte) {
            cevap["durum"] = "hata";
            cevap["mesaj"] = "Bu uye ayni kitabi zaten odunc almis.";
            return cevap;
        }
    }

    kitap.kopya_sayisi -= 1;
    kitaplar.guncelle(isbn, kitap);

    OduncKaydi yeniKayit{
        sonrakiKayitId++,
        uyeNo,
        isbn,
        QDate::currentDate().toString("yyyy-MM-dd").toStdString(),
        std::nullopt,
        OduncDurum::Oduncte
    };

    oduncler.ekle(yeniKayit.kayit_id, yeniKayit);

    cevap["durum"] = "basarili";
    cevap["mesaj"] = "Kitap odunc verildi.";
    cevap["kayit_id"] = yeniKayit.kayit_id;
    cevap["kayit"] = oduncJson(yeniKayit);

    return cevap;
}

QJsonObject KutuphaneSunucu::iadeEt(const QJsonObject& istek) {
    const int kayitId = istek["kayit_id"].toInt();

    std::lock_guard<std::mutex> kilit(veriMutex);

    QJsonObject cevap;

    auto kayitOpt = oduncler.bul(kayitId);

    if (!kayitOpt.has_value()) {
        cevap["durum"] = "hata";
        cevap["mesaj"] = "Odunc kaydi bulunamadi.";
        return cevap;
    }

    OduncKaydi kayit = kayitOpt.value();

    if (kayit.durum == OduncDurum::IadeEdildi) {
        cevap["durum"] = "hata";
        cevap["mesaj"] = "Bu kitap zaten iade edilmis.";
        return cevap;
    }

    kayit.durum = OduncDurum::IadeEdildi;
    kayit.iade_tarihi = QDate::currentDate().toString("yyyy-MM-dd").toStdString();

    oduncler.guncelle(kayitId, kayit);

    auto kitapOpt = kitaplar.bul(kayit.isbn);

    if (kitapOpt.has_value()) {
        Kitap kitap = kitapOpt.value();
        kitap.kopya_sayisi += 1;

        kitaplar.guncelle(kitap.isbn, kitap);
    }

    cevap["durum"] = "basarili";
    cevap["mesaj"] = "Kitap iade edildi.";
    cevap["kayit"] = oduncJson(kayit);

    return cevap;
}

QJsonObject KutuphaneSunucu::kitapJson(const Kitap& kitap) const {
    QJsonObject nesne;
    nesne["isbn"] = QString::fromStdString(kitap.isbn);
    nesne["baslik"] = QString::fromStdString(kitap.baslik);
    nesne["yazar"] = QString::fromStdString(kitap.yazar);
    nesne["yayin_yili"] = kitap.yayin_yili;
    nesne["kategori"] = QString::fromStdString(kitap.kategori);
    nesne["kopya_sayisi"] = kitap.kopya_sayisi;
    return nesne;
}

QJsonObject KutuphaneSunucu::oduncJson(const OduncKaydi& kayit) const {
    QJsonObject nesne;
    nesne["kayit_id"] = kayit.kayit_id;
    nesne["uye_no"] = kayit.uye_no;
    nesne["isbn"] = QString::fromStdString(kayit.isbn);
    nesne["odunc_tarihi"] = QString::fromStdString(kayit.odunc_tarihi);
    nesne["iade_tarihi"] = QString::fromStdString(kayit.iade_tarihi.value_or(""));
    nesne["durum"] = QString::fromStdString(durumYazisi(kayit.durum));
    return nesne;
}