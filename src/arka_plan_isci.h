#pragma once

#include "../include/varliklar.h"

#include <QObject>
#include <QString>

#include <atomic>
#include <mutex>
#include <vector>

using KitapListesi = std::vector<Kitap>;
using OduncListesi = std::vector<OduncKaydi>;

Q_DECLARE_METATYPE(KitapListesi)
Q_DECLARE_METATYPE(OduncListesi)

class ArkaPlanIsci : public QObject {
    Q_OBJECT

public:
    explicit ArkaPlanIsci(QObject* parent = nullptr);

    void iptalEt();

public slots:
    void csvKatalogYukle(const QString& dosyaYolu);
    void gecikmeRaporuOlustur(OduncListesi oduncler);

signals:
    void ilerlemeGuncellendi(int yuzde);
    void katalogYuklemeTamamlandi(KitapListesi kitaplar);
    void raporTamamlandi(const QString& rapor);
    void iptalEdildi();
    void hataOlustu(const QString& mesaj);

private:
    std::atomic_bool iptalIstendi{false};
    std::mutex veriMutex;

    bool satirdanKitapOlustur(const QString& satir, Kitap& kitap) const;
};