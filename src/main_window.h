#pragma once

#include "../include/depo.h"
#include "../include/varliklar.h"

#include "arka_plan_isci.h"
#include "kutuphane_sunucu.h"
#include "ag_istemci.h"
#include "kitap_model.h"
#include "uye_model.h"
#include "odunc_model.h"

#include <QMainWindow>
#include <QString>

#include <string>
#include <vector>

class QAction;
class QLabel;
class QLineEdit;
class QProgressBar;
class QSortFilterProxyModel;
class QTableView;
class QTabWidget;
class QChartView;
class QCloseEvent;
class QThread;
class QJsonObject;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);

protected:
    void closeEvent(QCloseEvent* event) override;

private:
    Depo<std::string, Kitap> kitaplar;
    Depo<int, Uye> uyeler;
    Depo<int, OduncKaydi> oduncler;

    KitapModel* kitapModel{};
    UyeModel* uyeModel{};
    OduncModel* oduncModel{};

    QSortFilterProxyModel* kitapProxy{};
    QSortFilterProxyModel* uyeProxy{};
    QSortFilterProxyModel* oduncProxy{};

    QTabWidget* sekmeler{};

    QTableView* kitapTablosu{};
    QTableView* uyeTablosu{};
    QTableView* oduncTablosu{};

    QLineEdit* kitapAramaKutusu{};
    QLineEdit* uyeAramaKutusu{};
    QLineEdit* oduncAramaKutusu{};

    QLabel* durumEtiketi{};
    QChartView* grafikGorunumu{};
    QProgressBar* ilerlemeCubugu{};

    QAction* islemIptalEylemi{};

    QThread* arkaPlanThread{};
    ArkaPlanIsci* arkaPlanIsci{};

    KutuphaneSunucu* kutuphaneSunucu{};
    AgIstemci* agIstemci{};

    void ornekVerileriOlustur();
    void modelleriKur();
    void agModulleriniKur();

    void arayuzuKur();
    void menuKur();
    void aracCubuguKur();
    void durumCubuguKur();
    void sekmeleriKur();

    void kitapSekmesiniKur();
    void uyeSekmesiniKur();
    void oduncSekmesiniKur();
    void grafikSekmesiniKur();

    void kitapEkle();
    void uyeEkle();
    void oduncVer();
    void oduncIadeEt();

    void verileriModellereAktar();
    void grafikYenile();
    void durumBilgisiniYenile();

    std::vector<Kitap> kitapVektoru() const;
    std::vector<Uye> uyeVektoru() const;
    std::vector<OduncKaydi> oduncVektoru() const;

    int sonrakiOduncKayitId() const;

    void jsonKaydet();
    void jsonYukle();
    bool jsonDosyasinaYaz(const QString& dosyaYolu);
    bool jsonDosyasindanOku(const QString& dosyaYolu);

    void binaryKaydet();
    void binaryYukle();
    bool binaryDosyasinaYaz(const QString& dosyaYolu);
    bool binaryDosyasindanOku(const QString& dosyaYolu);
    QString varsayilanBinaryDosyaYolu() const;

    void csvKatalogYukle();
    void gecikmeRaporuOlustur();
    void arkaPlanIsleminiIptalEt();

    void arkaPlanBasladi(const QString& mesaj);
    void arkaPlanBitti();

    void katalogYuklemeTamamlandi(const KitapListesi& yeniKitaplar);
    void gecikmeRaporuTamamlandi(const QString& rapor);
    void arkaPlanHatasi(const QString& mesaj);
    void arkaPlanIptalEdildi();

    void sunucuyuBaslat();
    void sunucuyuDurdur();
    void sunucuyaBaglan();
    void sunucuBaglantisiniKes();
    void agKimlikDogrula();
    void uzakKitapAra();
    void uzakOduncAl();
    void uzakIadeEt();
    void agCevabiIsle(const QJsonObject& cevap);

    void pencereAyarlariniYukle();
    void pencereAyarlariniKaydet();
};