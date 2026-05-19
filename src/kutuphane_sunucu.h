#pragma once

#include "../include/depo.h"
#include "../include/varliklar.h"

#include <QObject>
#include <QJsonObject>
#include <QList>
#include <QTcpServer>
#include <QTcpSocket>

#include <mutex>
#include <string>

class KutuphaneSunucu : public QObject {
    Q_OBJECT

public:
    explicit KutuphaneSunucu(QObject* parent = nullptr);

    bool baslat(quint16 port);
    void durdur();
    bool calisiyorMu() const;

signals:
    void durumMesaji(const QString& mesaj);
    void istemciSayisiDegisti(int sayi);

private slots:
    void yeniBaglantiGeldi();
    void istemcidenVeriGeldi();
    void istemciAyrildi();

private:
    QTcpServer* sunucu{};
    QList<QTcpSocket*> istemciler;

    Depo<std::string, Kitap> kitaplar;
    Depo<int, Uye> uyeler;
    Depo<int, OduncKaydi> oduncler;

    int sonrakiKayitId{1};
    std::mutex veriMutex;

    void ornekVerileriOlustur();

    void istekIsle(QTcpSocket* istemci, const QJsonObject& istek);
    void cevapGonder(QTcpSocket* istemci, const QJsonObject& cevap);

    QJsonObject kimlikDogrula(const QJsonObject& istek);
    QJsonObject kitapAra(const QJsonObject& istek);
    QJsonObject oduncAl(const QJsonObject& istek);
    QJsonObject iadeEt(const QJsonObject& istek);

    QJsonObject kitapJson(const Kitap& kitap) const;
    QJsonObject oduncJson(const OduncKaydi& kayit) const;
};