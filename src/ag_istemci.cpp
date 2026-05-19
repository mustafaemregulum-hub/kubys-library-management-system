#include "ag_istemci.h"

#include <QJsonDocument>
#include <QJsonParseError>

AgIstemci::AgIstemci(QObject* parent)
    : QObject(parent),
      soket(new QTcpSocket(this)),
      yenidenBaglanmaZamani(new QTimer(this))
{
    yenidenBaglanmaZamani->setInterval(3000);

    connect(soket, &QTcpSocket::connected, this, [this]() {
        // yenidenBaglanmaAktif baglan() fonksiyonunda zaten true olarak set ediliyor;
        // burada tekrar set etmek gereksiz ve yanilticiydi.
        yenidenBaglanmaZamani->stop();

        emit baglandi();
        emit durumMesaji("Sunucuya baglanildi.");
    });

    connect(soket, &QTcpSocket::disconnected, this, [this]() {
        emit baglantiKesildi();
        emit durumMesaji("Sunucu baglantisi kesildi.");

        if (yenidenBaglanmaAktif && !sonHost.isEmpty() && sonPort != 0) {
            yenidenBaglanmaZamani->start();
        }
    });

    connect(soket, &QTcpSocket::readyRead, this, &AgIstemci::veriGeldi);
    connect(soket, &QTcpSocket::errorOccurred, this, &AgIstemci::soketHatasiOlustu);

    connect(
        yenidenBaglanmaZamani,
        &QTimer::timeout,
        this,
        &AgIstemci::yenidenBaglanmayiDene
    );
}

void AgIstemci::baglan(const QString& host, quint16 port) {
    sonHost = host;
    sonPort = port;
    yenidenBaglanmaAktif = true;

    if (soket->state() != QAbstractSocket::UnconnectedState) {
        soket->abort();
    }

    emit durumMesaji(QString("Sunucuya baglaniliyor: %1:%2").arg(host).arg(port));
    soket->connectToHost(host, port);
}

void AgIstemci::baglantiyiKes() {
    yenidenBaglanmaAktif = false;
    yenidenBaglanmaZamani->stop();

    if (soket->state() != QAbstractSocket::UnconnectedState) {
        soket->disconnectFromHost();
    }
}

bool AgIstemci::bagliMi() const {
    return soket->state() == QAbstractSocket::ConnectedState;
}

void AgIstemci::kimlikDogrula(const QString& kullanici, const QString& sifre) {
    QJsonObject istek;
    istek["tip"] = "kimlik_dogrula";
    istek["kullanici"] = kullanici;
    istek["sifre"] = sifre;

    istekGonder(istek);
}

void AgIstemci::kitapAra(const QString& aranan) {
    QJsonObject istek;
    istek["tip"] = "kitap_ara";
    istek["aranan"] = aranan;

    istekGonder(istek);
}

void AgIstemci::oduncAl(int uyeNo, const QString& isbn) {
    QJsonObject istek;
    istek["tip"] = "odunc_al";
    istek["uye_no"] = uyeNo;
    istek["isbn"] = isbn;

    istekGonder(istek);
}

void AgIstemci::iadeEt(int kayitId) {
    QJsonObject istek;
    istek["tip"] = "iade_et";
    istek["kayit_id"] = kayitId;

    istekGonder(istek);
}

void AgIstemci::istekGonder(const QJsonObject& istek) {
    if (!bagliMi()) {
        emit hataOlustu("Sunucuya bagli degilsiniz.");
        return;
    }

    const QJsonDocument belge(istek);

    soket->write(belge.toJson(QJsonDocument::Compact));
    soket->write("\n");
    soket->flush();
}

void AgIstemci::veriGeldi() {
    while (soket->canReadLine()) {
        const QByteArray satir = soket->readLine().trimmed();

        if (satir.isEmpty()) {
            continue;
        }

        QJsonParseError hata;
        const QJsonDocument belge = QJsonDocument::fromJson(satir, &hata);

        if (hata.error != QJsonParseError::NoError || !belge.isObject()) {
            emit hataOlustu("Sunucudan gecersiz JSON cevabi geldi.");
            continue;
        }

        emit cevapGeldi(belge.object());
    }
}

void AgIstemci::soketHatasiOlustu(QAbstractSocket::SocketError hata) {
    Q_UNUSED(hata)

    emit hataOlustu(soket->errorString());
}

void AgIstemci::yenidenBaglanmayiDene() {
    if (!yenidenBaglanmaAktif || sonHost.isEmpty() || sonPort == 0) {
        yenidenBaglanmaZamani->stop();
        return;
    }

    if (soket->state() == QAbstractSocket::ConnectedState ||
        soket->state() == QAbstractSocket::ConnectingState) {
        return;
    }

    emit durumMesaji("Sunucuya yeniden baglanma deneniyor...");
    soket->connectToHost(sonHost, sonPort);
}