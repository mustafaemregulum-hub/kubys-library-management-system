#pragma once

#include <QObject>
#include <QJsonObject>
#include <QTcpSocket>
#include <QTimer>

class AgIstemci : public QObject {
    Q_OBJECT

public:
    explicit AgIstemci(QObject* parent = nullptr);

    void baglan(const QString& host, quint16 port);
    void baglantiyiKes();

    bool bagliMi() const;

    void kimlikDogrula(const QString& kullanici, const QString& sifre);
    void kitapAra(const QString& aranan);
    void oduncAl(int uyeNo, const QString& isbn);
    void iadeEt(int kayitId);

signals:
    void baglandi();
    void baglantiKesildi();
    void hataOlustu(const QString& mesaj);
    void cevapGeldi(const QJsonObject& cevap);
    void durumMesaji(const QString& mesaj);

private slots:
    void veriGeldi();
    void soketHatasiOlustu(QAbstractSocket::SocketError hata);
    void yenidenBaglanmayiDene();

private:
    QTcpSocket* soket{};
    QTimer* yenidenBaglanmaZamani{};

    QString sonHost;
    quint16 sonPort{0};
    bool yenidenBaglanmaAktif{false};

    void istekGonder(const QJsonObject& istek);
};