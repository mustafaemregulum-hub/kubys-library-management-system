#include "main_window.h"

#include "kitap_dialog.h"
#include "uye_dialog.h"
#include "odunc_dialog.h"

#include "dosya_islemleri.h"
#include "faz1_sorgular.h"

#include <QAbstractItemView>
#include <QAction>
#include <QChart>
#include <QChartView>
#include <QCloseEvent>
#include <QCoreApplication>
#include <QDate>
#include <QDialog>
#include <QFile>
#include <QFileDialog>
#include <QHeaderView>
#include <QInputDialog>
#include <QItemSelectionModel>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QLabel>
#include <QLegend>
#include <QLineEdit>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QMetaType>
#include <QPainter>
#include <QPieSeries>
#include <QProgressBar>
#include <QSettings>
#include <QSortFilterProxyModel>
#include <QStatusBar>
#include <QTableView>
#include <QTabWidget>
#include <QThread>
#include <QToolBar>
#include <QVBoxLayout>
#include <QWidget>

#include <algorithm>
#include <map>
#include <ranges>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    qRegisterMetaType<KitapListesi>("KitapListesi");
    qRegisterMetaType<OduncListesi>("OduncListesi");

    if (!binaryDosyasindanOku(varsayilanBinaryDosyaYolu())) {
        ornekVerileriOlustur();
    }

    modelleriKur();
    arayuzuKur();
    agModulleriniKur();
    verileriModellereAktar();
    pencereAyarlariniYukle();
}

void MainWindow::ornekVerileriOlustur() {
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

    oduncler.ekle(4, {
        4,
        2003,
        "978-975-1000-01-1",
        "2025-03-08",
        std::nullopt,
        OduncDurum::Oduncte
    });
}

void MainWindow::modelleriKur() {
    kitapModel = new KitapModel(this);
    uyeModel = new UyeModel(this);
    oduncModel = new OduncModel(this);

    kitapProxy = new QSortFilterProxyModel(this);
    kitapProxy->setSourceModel(kitapModel);
    kitapProxy->setFilterCaseSensitivity(Qt::CaseInsensitive);
    kitapProxy->setFilterKeyColumn(-1);

    uyeProxy = new QSortFilterProxyModel(this);
    uyeProxy->setSourceModel(uyeModel);
    uyeProxy->setFilterCaseSensitivity(Qt::CaseInsensitive);
    uyeProxy->setFilterKeyColumn(-1);

    oduncProxy = new QSortFilterProxyModel(this);
    oduncProxy->setSourceModel(oduncModel);
    oduncProxy->setFilterCaseSensitivity(Qt::CaseInsensitive);
    oduncProxy->setFilterKeyColumn(-1);
}

void MainWindow::agModulleriniKur() {
    kutuphaneSunucu = new KutuphaneSunucu(this);
    agIstemci = new AgIstemci(this);

    connect(kutuphaneSunucu, &KutuphaneSunucu::durumMesaji, this, [this](const QString& mesaj) {
        statusBar()->showMessage(mesaj, 4000);
    });

    connect(kutuphaneSunucu, &KutuphaneSunucu::istemciSayisiDegisti, this, [this](int sayi) {
        statusBar()->showMessage(QString("Bagli istemci sayisi: %1").arg(sayi), 4000);
    });

    connect(agIstemci, &AgIstemci::baglandi, this, [this]() {
        QMessageBox::information(this, "Ag", "Sunucuya baglanildi.");
    });

    connect(agIstemci, &AgIstemci::baglantiKesildi, this, [this]() {
        statusBar()->showMessage("Sunucu baglantisi kesildi.", 4000);
    });

    connect(agIstemci, &AgIstemci::hataOlustu, this, [this](const QString& mesaj) {
        QMessageBox::warning(this, "Ag Hatasi", mesaj);
    });

    connect(agIstemci, &AgIstemci::durumMesaji, this, [this](const QString& mesaj) {
        statusBar()->showMessage(mesaj, 4000);
    });

    connect(agIstemci, &AgIstemci::cevapGeldi, this, &MainWindow::agCevabiIsle);
}

void MainWindow::arayuzuKur() {
    setWindowTitle("KUBYS - Kutuphane Yonetim Sistemi");
    resize(1100, 700);

    sekmeler = new QTabWidget(this);
    setCentralWidget(sekmeler);

    menuKur();
    aracCubuguKur();
    sekmeleriKur();
    durumCubuguKur();
}

void MainWindow::menuKur() {
    QMenu* dosyaMenusu = menuBar()->addMenu("Dosya");

    QAction* kaydetEylemi = new QAction("Kaydet", this);
    connect(kaydetEylemi, &QAction::triggered, this, &MainWindow::jsonKaydet);
    dosyaMenusu->addAction(kaydetEylemi);

    QAction* yukleEylemi = new QAction("Yukle", this);
    connect(yukleEylemi, &QAction::triggered, this, &MainWindow::jsonYukle);
    dosyaMenusu->addAction(yukleEylemi);

    QAction* binaryKaydetEylemi = new QAction("Binary Kaydet", this);
    connect(binaryKaydetEylemi, &QAction::triggered, this, &MainWindow::binaryKaydet);
    dosyaMenusu->addAction(binaryKaydetEylemi);

    QAction* binaryYukleEylemi = new QAction("Binary Yukle", this);
    connect(binaryYukleEylemi, &QAction::triggered, this, &MainWindow::binaryYukle);
    dosyaMenusu->addAction(binaryYukleEylemi);

    dosyaMenusu->addSeparator();

    QAction* csvYukleEylemi = new QAction("CSV Katalog Yukle", this);
    connect(csvYukleEylemi, &QAction::triggered, this, &MainWindow::csvKatalogYukle);
    dosyaMenusu->addAction(csvYukleEylemi);

    QAction* gecikmeRaporuEylemi = new QAction("Gecikme Raporu Olustur", this);
    connect(gecikmeRaporuEylemi, &QAction::triggered, this, &MainWindow::gecikmeRaporuOlustur);
    dosyaMenusu->addAction(gecikmeRaporuEylemi);

    dosyaMenusu->addSeparator();

    QAction* cikisEylemi = new QAction("Cikis", this);
    connect(cikisEylemi, &QAction::triggered, this, &QWidget::close);
    dosyaMenusu->addAction(cikisEylemi);

    QMenu* duzenMenusu = menuBar()->addMenu("Duzen");

    QAction* kitapEkleEylemi = new QAction("Kitap Ekle", this);
    connect(kitapEkleEylemi, &QAction::triggered, this, &MainWindow::kitapEkle);
    duzenMenusu->addAction(kitapEkleEylemi);

    QAction* uyeEkleEylemi = new QAction("Uye Ekle", this);
    connect(uyeEkleEylemi, &QAction::triggered, this, &MainWindow::uyeEkle);
    duzenMenusu->addAction(uyeEkleEylemi);

    QAction* oduncVerEylemi = new QAction("Odunc Ver", this);
    connect(oduncVerEylemi, &QAction::triggered, this, &MainWindow::oduncVer);
    duzenMenusu->addAction(oduncVerEylemi);

    QAction* iadeEtEylemi = new QAction("Iade Et", this);
    connect(iadeEtEylemi, &QAction::triggered, this, &MainWindow::oduncIadeEt);
    duzenMenusu->addAction(iadeEtEylemi);

    QMenu* agMenusu = menuBar()->addMenu("Ag");

    QAction* sunucuBaslatEylemi = new QAction("Sunucuyu Baslat", this);
    connect(sunucuBaslatEylemi, &QAction::triggered, this, &MainWindow::sunucuyuBaslat);
    agMenusu->addAction(sunucuBaslatEylemi);

    QAction* sunucuDurdurEylemi = new QAction("Sunucuyu Durdur", this);
    connect(sunucuDurdurEylemi, &QAction::triggered, this, &MainWindow::sunucuyuDurdur);
    agMenusu->addAction(sunucuDurdurEylemi);

    agMenusu->addSeparator();

    QAction* baglanEylemi = new QAction("Sunucuya Baglan", this);
    connect(baglanEylemi, &QAction::triggered, this, &MainWindow::sunucuyaBaglan);
    agMenusu->addAction(baglanEylemi);

    QAction* baglantiKesEylemi = new QAction("Baglantiyi Kes", this);
    connect(baglantiKesEylemi, &QAction::triggered, this, &MainWindow::sunucuBaglantisiniKes);
    agMenusu->addAction(baglantiKesEylemi);

    QAction* kimlikEylemi = new QAction("Kimlik Dogrula", this);
    connect(kimlikEylemi, &QAction::triggered, this, &MainWindow::agKimlikDogrula);
    agMenusu->addAction(kimlikEylemi);

    agMenusu->addSeparator();

    QAction* uzakKitapAraEylemi = new QAction("Uzak Kitap Ara", this);
    connect(uzakKitapAraEylemi, &QAction::triggered, this, &MainWindow::uzakKitapAra);
    agMenusu->addAction(uzakKitapAraEylemi);

    QAction* uzakOduncAlEylemi = new QAction("Uzak Odunc Al", this);
    connect(uzakOduncAlEylemi, &QAction::triggered, this, &MainWindow::uzakOduncAl);
    agMenusu->addAction(uzakOduncAlEylemi);

    QAction* uzakIadeEtEylemi = new QAction("Uzak Iade Et", this);
    connect(uzakIadeEtEylemi, &QAction::triggered, this, &MainWindow::uzakIadeEt);
    agMenusu->addAction(uzakIadeEtEylemi);

    QMenu* yardimMenusu = menuBar()->addMenu("Yardim");

    QAction* hakkindaEylemi = new QAction("Hakkinda", this);
    connect(hakkindaEylemi, &QAction::triggered, this, [this]() {
        QMessageBox::about(
            this,
            "KUBYS Hakkinda",
            "KUBYS Kutuphane Yonetim Sistemi\nFaz 4 - Qt Network TCP Sunucu/Istemci"
        );
    });
    yardimMenusu->addAction(hakkindaEylemi);
}

void MainWindow::aracCubuguKur() {
    QToolBar* aracCubugu = addToolBar("Arac Cubugu");

    QAction* kitapEkleEylemi = new QAction("Kitap Ekle", this);
    connect(kitapEkleEylemi, &QAction::triggered, this, &MainWindow::kitapEkle);
    aracCubugu->addAction(kitapEkleEylemi);

    QAction* uyeEkleEylemi = new QAction("Uye Ekle", this);
    connect(uyeEkleEylemi, &QAction::triggered, this, &MainWindow::uyeEkle);
    aracCubugu->addAction(uyeEkleEylemi);

    QAction* oduncVerEylemi = new QAction("Odunc Ver", this);
    connect(oduncVerEylemi, &QAction::triggered, this, &MainWindow::oduncVer);
    aracCubugu->addAction(oduncVerEylemi);

    QAction* iadeEtEylemi = new QAction("Iade Et", this);
    connect(iadeEtEylemi, &QAction::triggered, this, &MainWindow::oduncIadeEt);
    aracCubugu->addAction(iadeEtEylemi);

    aracCubugu->addSeparator();

    QAction* kaydetEylemi = new QAction("Kaydet", this);
    connect(kaydetEylemi, &QAction::triggered, this, &MainWindow::jsonKaydet);
    aracCubugu->addAction(kaydetEylemi);

    aracCubugu->addSeparator();

    QAction* csvYukleEylemi = new QAction("CSV Yukle", this);
    connect(csvYukleEylemi, &QAction::triggered, this, &MainWindow::csvKatalogYukle);
    aracCubugu->addAction(csvYukleEylemi);

    QAction* raporEylemi = new QAction("Gecikme Raporu", this);
    connect(raporEylemi, &QAction::triggered, this, &MainWindow::gecikmeRaporuOlustur);
    aracCubugu->addAction(raporEylemi);

    islemIptalEylemi = new QAction("Iptal", this);
    islemIptalEylemi->setEnabled(false);
    connect(islemIptalEylemi, &QAction::triggered, this, &MainWindow::arkaPlanIsleminiIptalEt);
    aracCubugu->addAction(islemIptalEylemi);

    aracCubugu->addSeparator();

    QAction* sunucuBaslatEylemi = new QAction("Sunucu Baslat", this);
    connect(sunucuBaslatEylemi, &QAction::triggered, this, &MainWindow::sunucuyuBaslat);
    aracCubugu->addAction(sunucuBaslatEylemi);

    QAction* baglanEylemi = new QAction("Baglan", this);
    connect(baglanEylemi, &QAction::triggered, this, &MainWindow::sunucuyaBaglan);
    aracCubugu->addAction(baglanEylemi);

    QAction* araEylemi = new QAction("Uzak Ara", this);
    connect(araEylemi, &QAction::triggered, this, &MainWindow::uzakKitapAra);
    aracCubugu->addAction(araEylemi);
}

void MainWindow::durumCubuguKur() {
    ilerlemeCubugu = new QProgressBar(this);
    ilerlemeCubugu->setRange(0, 100);
    ilerlemeCubugu->setValue(0);
    ilerlemeCubugu->setVisible(false);
    ilerlemeCubugu->setFixedWidth(180);

    durumEtiketi = new QLabel(this);

    statusBar()->addPermanentWidget(ilerlemeCubugu);
    statusBar()->addPermanentWidget(durumEtiketi);

    durumBilgisiniYenile();
}

void MainWindow::sekmeleriKur() {
    kitapSekmesiniKur();
    uyeSekmesiniKur();
    oduncSekmesiniKur();
    grafikSekmesiniKur();
}

void MainWindow::kitapSekmesiniKur() {
    QWidget* sekme = new QWidget(this);
    QVBoxLayout* yerlesim = new QVBoxLayout(sekme);

    kitapAramaKutusu = new QLineEdit(sekme);
    kitapAramaKutusu->setPlaceholderText("Kitaplarda ara: ISBN, baslik, yazar veya kategori");
    yerlesim->addWidget(kitapAramaKutusu);

    kitapTablosu = new QTableView(sekme);
    kitapTablosu->setModel(kitapProxy);
    kitapTablosu->setSortingEnabled(true);
    kitapTablosu->setSelectionBehavior(QAbstractItemView::SelectRows);
    kitapTablosu->setSelectionMode(QAbstractItemView::SingleSelection);
    kitapTablosu->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    yerlesim->addWidget(kitapTablosu);

    connect(
        kitapAramaKutusu,
        &QLineEdit::textChanged,
        kitapProxy,
        &QSortFilterProxyModel::setFilterFixedString
    );

    sekmeler->addTab(sekme, "Kitaplar");
}

void MainWindow::uyeSekmesiniKur() {
    QWidget* sekme = new QWidget(this);
    QVBoxLayout* yerlesim = new QVBoxLayout(sekme);

    uyeAramaKutusu = new QLineEdit(sekme);
    uyeAramaKutusu->setPlaceholderText("Uyelerde ara: uye no, isim, soyisim veya telefon");
    yerlesim->addWidget(uyeAramaKutusu);

    uyeTablosu = new QTableView(sekme);
    uyeTablosu->setModel(uyeProxy);
    uyeTablosu->setSortingEnabled(true);
    uyeTablosu->setSelectionBehavior(QAbstractItemView::SelectRows);
    uyeTablosu->setSelectionMode(QAbstractItemView::SingleSelection);
    uyeTablosu->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    yerlesim->addWidget(uyeTablosu);

    connect(
        uyeAramaKutusu,
        &QLineEdit::textChanged,
        uyeProxy,
        &QSortFilterProxyModel::setFilterFixedString
    );

    sekmeler->addTab(sekme, "Uyeler");
}

void MainWindow::oduncSekmesiniKur() {
    QWidget* sekme = new QWidget(this);
    QVBoxLayout* yerlesim = new QVBoxLayout(sekme);

    oduncAramaKutusu = new QLineEdit(sekme);
    oduncAramaKutusu->setPlaceholderText("Odunc kayitlarinda ara");
    yerlesim->addWidget(oduncAramaKutusu);

    oduncTablosu = new QTableView(sekme);
    oduncTablosu->setModel(oduncProxy);
    oduncTablosu->setSortingEnabled(true);
    oduncTablosu->setSelectionBehavior(QAbstractItemView::SelectRows);
    oduncTablosu->setSelectionMode(QAbstractItemView::SingleSelection);
    oduncTablosu->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    yerlesim->addWidget(oduncTablosu);

    connect(
        oduncAramaKutusu,
        &QLineEdit::textChanged,
        oduncProxy,
        &QSortFilterProxyModel::setFilterFixedString
    );

    sekmeler->addTab(sekme, "Odunc Kayitlari");
}

void MainWindow::grafikSekmesiniKur() {
    QWidget* sekme = new QWidget(this);
    QVBoxLayout* yerlesim = new QVBoxLayout(sekme);

    grafikGorunumu = new QChartView(sekme);
    grafikGorunumu->setRenderHint(QPainter::Antialiasing);

    yerlesim->addWidget(grafikGorunumu);

    sekmeler->addTab(sekme, "Grafik");
}

void MainWindow::kitapEkle() {
    KitapDialog dialog(this);

    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    const Kitap yeniKitap = dialog.kitap();

    if (!kitaplar.ekle(yeniKitap.isbn, yeniKitap)) {
        QMessageBox::warning(this, "Kayit Var", "Bu ISBN ile zaten bir kitap kayitli.");
        return;
    }

    verileriModellereAktar();
    statusBar()->showMessage("Kitap eklendi.", 3000);
}

void MainWindow::uyeEkle() {
    UyeDialog dialog(this);

    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    const Uye yeniUye = dialog.uye();

    if (!uyeler.ekle(yeniUye.uye_no, yeniUye)) {
        QMessageBox::warning(this, "Kayit Var", "Bu uye numarasi ile zaten bir uye kayitli.");
        return;
    }

    verileriModellereAktar();
    statusBar()->showMessage("Uye eklendi.", 3000);
}

void MainWindow::oduncVer() {
    OduncDialog dialog(uyeVektoru(), kitapVektoru(), sonrakiOduncKayitId(), this);

    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    const OduncKaydi yeniKayit = dialog.kayit();

    if (!uyeler.var_mi(yeniKayit.uye_no)) {
        QMessageBox::warning(this, "Uye Bulunamadi", "Secilen uye sistemde bulunamadi.");
        return;
    }

    auto kitapOpt = kitaplar.bul(yeniKayit.isbn);

    if (!kitapOpt.has_value()) {
        QMessageBox::warning(this, "Kitap Bulunamadi", "Secilen kitap sistemde bulunamadi.");
        return;
    }

    Kitap kitap = kitapOpt.value();

    if (kitap.kopya_sayisi <= 0) {
        QMessageBox::warning(this, "Kopya Yok", "Bu kitabin odunc verilebilecek uygun kopyasi yok.");
        return;
    }

    for (const auto& kayit : oduncler.tumunu_al() | std::views::values) {
        if (kayit.uye_no == yeniKayit.uye_no
            && kayit.isbn == yeniKayit.isbn
            && kayit.durum == OduncDurum::Oduncte) {
            QMessageBox::warning(this, "Tekrarli Odunc", "Bu uye ayni kitabi zaten odunc almis.");
            return;
        }
    }

    if (!oduncler.ekle(yeniKayit.kayit_id, yeniKayit)) {
        QMessageBox::warning(this, "Kayit Var", "Bu odunc kayit numarasi zaten mevcut.");
        return;
    }

    kitap.kopya_sayisi -= 1;
    kitaplar.guncelle(kitap.isbn, kitap);

    verileriModellereAktar();
    statusBar()->showMessage("Odunc kaydi eklendi ve kitap kopya sayisi guncellendi.", 3000);
}

void MainWindow::oduncIadeEt() {
    int varsayilanKayitId = std::max(1, sonrakiOduncKayitId() - 1);

    if (oduncTablosu && oduncTablosu->selectionModel()) {
        const QModelIndexList seciliSatirlar = oduncTablosu->selectionModel()->selectedRows();

        if (!seciliSatirlar.isEmpty()) {
            const QModelIndex kaynakIndeks = oduncProxy->mapToSource(seciliSatirlar.first());
            varsayilanKayitId = oduncModel->index(kaynakIndeks.row(), 0).data().toInt();
        }
    }

    bool tamam = false;
    const int kayitId = QInputDialog::getInt(
        this,
        "Iade Et",
        "Odunc Kayit ID:",
        varsayilanKayitId,
        1,
        999999,
        1,
        &tamam
    );

    if (!tamam) {
        return;
    }

    auto kayitOpt = oduncler.bul(kayitId);

    if (!kayitOpt.has_value()) {
        QMessageBox::warning(this, "Kayit Bulunamadi", "Bu odunc kayit numarasi bulunamadi.");
        return;
    }

    OduncKaydi kayit = kayitOpt.value();

    if (kayit.durum == OduncDurum::IadeEdildi) {
        QMessageBox::warning(this, "Zaten Iade Edildi", "Bu kitap daha once iade edilmis.");
        return;
    }

    kayit.durum = OduncDurum::IadeEdildi;
    kayit.iade_tarihi = QDate::currentDate().toString("yyyy-MM-dd").toStdString();

    if (!oduncler.guncelle(kayitId, kayit)) {
        QMessageBox::warning(this, "Hata", "Odunc kaydi guncellenemedi.");
        return;
    }

    if (auto kitapOpt = kitaplar.bul(kayit.isbn)) {
        Kitap kitap = kitapOpt.value();
        kitap.kopya_sayisi += 1;
        kitaplar.guncelle(kitap.isbn, kitap);
    }

    verileriModellereAktar();
    statusBar()->showMessage("Kitap iade edildi ve kopya sayisi guncellendi.", 3000);
}

void MainWindow::verileriModellereAktar() {
    kitapModel->kitaplariAyarla(kitapVektoru());
    uyeModel->uyeleriAyarla(uyeVektoru());
    oduncModel->oduncleriAyarla(oduncVektoru());

    grafikYenile();
    durumBilgisiniYenile();
}

void MainWindow::grafikYenile() {
    if (!grafikGorunumu) {
        return;
    }

    std::map<std::string, int> kategoriSayilari;

    for (const auto& kitap : kitaplar.tumunu_al() | std::views::values) {
        ++kategoriSayilari[kitap.kategori];
    }

    QPieSeries* seri = new QPieSeries();

    for (const auto& [kategori, sayi] : kategoriSayilari) {
        seri->append(QString::fromStdString(kategori), sayi);
    }

    QChart* grafik = new QChart();
    grafik->addSeries(seri);
    grafik->setTitle("Kategoriye Gore Kitap Dagilimi");
    grafik->legend()->setVisible(true);

    grafikGorunumu->setChart(grafik);
}

void MainWindow::durumBilgisiniYenile() {
    if (!durumEtiketi) {
        return;
    }

    durumEtiketi->setText(
        QString("Kitap: %1 | Uye: %2 | Odunc Kaydi: %3")
            .arg(static_cast<qulonglong>(kitaplar.boyut()))
            .arg(static_cast<qulonglong>(uyeler.boyut()))
            .arg(static_cast<qulonglong>(oduncler.boyut()))
    );
}

std::vector<Kitap> MainWindow::kitapVektoru() const {
    std::vector<Kitap> sonuc;

    for (const auto& kitap : kitaplar.tumunu_al() | std::views::values) {
        sonuc.push_back(kitap);
    }

    return sonuc;
}

std::vector<Uye> MainWindow::uyeVektoru() const {
    std::vector<Uye> sonuc;

    for (const auto& uye : uyeler.tumunu_al() | std::views::values) {
        sonuc.push_back(uye);
    }

    return sonuc;
}

std::vector<OduncKaydi> MainWindow::oduncVektoru() const {
    std::vector<OduncKaydi> sonuc;

    for (const auto& kayit : oduncler.tumunu_al() | std::views::values) {
        sonuc.push_back(kayit);
    }

    return sonuc;
}

int MainWindow::sonrakiOduncKayitId() const {
    int enBuyuk = 0;

    for (const auto& kayit : oduncler.tumunu_al() | std::views::values) {
        enBuyuk = std::max(enBuyuk, kayit.kayit_id);
    }

    return enBuyuk + 1;
}

void MainWindow::jsonKaydet() {
    const QString dosyaYolu = QFileDialog::getSaveFileName(
        this,
        "JSON Kaydet",
        "kubys_veriler.json",
        "JSON Dosyalari (*.json)"
    );

    if (dosyaYolu.isEmpty()) {
        return;
    }

    if (jsonDosyasinaYaz(dosyaYolu)) {
        statusBar()->showMessage("JSON dosyasi kaydedildi.", 3000);
    } else {
        QMessageBox::warning(this, "Hata", "JSON dosyasi kaydedilemedi.");
    }
}

void MainWindow::jsonYukle() {
    const QString dosyaYolu = QFileDialog::getOpenFileName(
        this,
        "JSON Yukle",
        QString(),
        "JSON Dosyalari (*.json)"
    );

    if (dosyaYolu.isEmpty()) {
        return;
    }

    if (jsonDosyasindanOku(dosyaYolu)) {
        verileriModellereAktar();
        statusBar()->showMessage("JSON dosyasi yuklendi.", 3000);
    } else {
        QMessageBox::warning(this, "Hata", "JSON dosyasi yuklenemedi.");
    }
}

bool MainWindow::jsonDosyasinaYaz(const QString& dosyaYolu) {
    QJsonArray kitapDizisi;

    for (const auto& kitap : kitaplar.tumunu_al() | std::views::values) {
        QJsonObject nesne;
        nesne["isbn"] = QString::fromStdString(kitap.isbn);
        nesne["baslik"] = QString::fromStdString(kitap.baslik);
        nesne["yazar"] = QString::fromStdString(kitap.yazar);
        nesne["yayin_yili"] = kitap.yayin_yili;
        nesne["kategori"] = QString::fromStdString(kitap.kategori);
        nesne["kopya_sayisi"] = kitap.kopya_sayisi;
        kitapDizisi.append(nesne);
    }

    QJsonArray uyeDizisi;

    for (const auto& uye : uyeler.tumunu_al() | std::views::values) {
        QJsonObject nesne;
        nesne["uye_no"] = uye.uye_no;
        nesne["isim"] = QString::fromStdString(uye.isim);
        nesne["soyisim"] = QString::fromStdString(uye.soyisim);
        nesne["telefon"] = QString::fromStdString(uye.telefon);
        nesne["kayit_tarihi"] = QString::fromStdString(uye.kayit_tarihi);
        uyeDizisi.append(nesne);
    }

    QJsonArray oduncDizisi;

    for (const auto& kayit : oduncler.tumunu_al() | std::views::values) {
        QJsonObject nesne;
        nesne["kayit_id"] = kayit.kayit_id;
        nesne["uye_no"] = kayit.uye_no;
        nesne["isbn"] = QString::fromStdString(kayit.isbn);
        nesne["odunc_tarihi"] = QString::fromStdString(kayit.odunc_tarihi);
        nesne["iade_tarihi"] = QString::fromStdString(kayit.iade_tarihi.value_or(""));
        nesne["durum"] = QString::fromStdString(durumYazisi(kayit.durum));
        oduncDizisi.append(nesne);
    }

    QJsonObject kok;
    kok["kitaplar"] = kitapDizisi;
    kok["uyeler"] = uyeDizisi;
    kok["odunc_kayitlari"] = oduncDizisi;

    QFile dosya(dosyaYolu);

    if (!dosya.open(QIODevice::WriteOnly)) {
        return false;
    }

    QJsonDocument belge(kok);
    dosya.write(belge.toJson(QJsonDocument::Indented));
    dosya.close();

    return true;
}

bool MainWindow::jsonDosyasindanOku(const QString& dosyaYolu) {
    QFile dosya(dosyaYolu);

    if (!dosya.open(QIODevice::ReadOnly)) {
        return false;
    }

    const QByteArray veri = dosya.readAll();
    dosya.close();

    QJsonParseError hata;
    const QJsonDocument belge = QJsonDocument::fromJson(veri, &hata);

    if (hata.error != QJsonParseError::NoError || !belge.isObject()) {
        return false;
    }

    const QJsonObject kok = belge.object();

    Depo<std::string, Kitap> yeniKitaplar;
    Depo<int, Uye> yeniUyeler;
    Depo<int, OduncKaydi> yeniOduncler;

    for (const QJsonValue& deger : kok["kitaplar"].toArray()) {
        const QJsonObject nesne = deger.toObject();

        Kitap kitap{
            nesne["isbn"].toString().toStdString(),
            nesne["baslik"].toString().toStdString(),
            nesne["yazar"].toString().toStdString(),
            nesne["yayin_yili"].toInt(),
            nesne["kategori"].toString().toStdString(),
            nesne["kopya_sayisi"].toInt()
        };

        yeniKitaplar.ekle(kitap.isbn, kitap);
    }

    for (const QJsonValue& deger : kok["uyeler"].toArray()) {
        const QJsonObject nesne = deger.toObject();

        Uye uye{
            nesne["uye_no"].toInt(),
            nesne["isim"].toString().toStdString(),
            nesne["soyisim"].toString().toStdString(),
            nesne["telefon"].toString().toStdString(),
            nesne["kayit_tarihi"].toString().toStdString()
        };

        yeniUyeler.ekle(uye.uye_no, uye);
    }

    for (const QJsonValue& deger : kok["odunc_kayitlari"].toArray()) {
        const QJsonObject nesne = deger.toObject();

        const QString durumMetni = nesne["durum"].toString();
        OduncDurum durum = OduncDurum::Oduncte;

        if (durumMetni == "Iade Edildi") {
            durum = OduncDurum::IadeEdildi;
        } else if (durumMetni == "Gecikmis") {
            durum = OduncDurum::Gecikmis;
        }

        const QString iadeTarihi = nesne["iade_tarihi"].toString();

        OduncKaydi kayit{
            nesne["kayit_id"].toInt(),
            nesne["uye_no"].toInt(),
            nesne["isbn"].toString().toStdString(),
            nesne["odunc_tarihi"].toString().toStdString(),
            iadeTarihi.isEmpty()
                ? std::optional<std::string>{}
                : std::optional<std::string>{iadeTarihi.toStdString()},
            durum
        };

        yeniOduncler.ekle(kayit.kayit_id, kayit);
    }

    kitaplar = yeniKitaplar;
    uyeler = yeniUyeler;
    oduncler = yeniOduncler;

    return true;
}

void MainWindow::binaryKaydet() {
    const QString dosyaYolu = QFileDialog::getSaveFileName(
        this,
        "Binary Kaydet",
        "kubys_veriler.bin",
        "Binary Dosyalari (*.bin)"
    );

    if (dosyaYolu.isEmpty()) {
        return;
    }

    if (binaryDosyasinaYaz(dosyaYolu)) {
        statusBar()->showMessage("Binary dosya kaydedildi.", 3000);
    } else {
        QMessageBox::warning(this, "Hata", "Binary dosya kaydedilemedi.");
    }
}

void MainWindow::binaryYukle() {
    const QString dosyaYolu = QFileDialog::getOpenFileName(
        this,
        "Binary Yukle",
        QString(),
        "Binary Dosyalari (*.bin)"
    );

    if (dosyaYolu.isEmpty()) {
        return;
    }

    if (binaryDosyasindanOku(dosyaYolu)) {
        verileriModellereAktar();
        statusBar()->showMessage("Binary dosya yuklendi.", 3000);
    } else {
        QMessageBox::warning(this, "Hata", "Binary dosya okunamadi veya format gecersiz.");
    }
}

bool MainWindow::binaryDosyasinaYaz(const QString& dosyaYolu) {
    return kubys::dosya::kubys_verilerini_dosyaya_kaydet(
        dosyaYolu.toStdString(),
        kitaplar,
        uyeler,
        oduncler
    );
}

bool MainWindow::binaryDosyasindanOku(const QString& dosyaYolu) {
    const auto paket = kubys::dosya::kubys_verilerini_dosyadan_oku(dosyaYolu.toStdString());

    if (!paket.has_value()) {
        return false;
    }

    kitaplar = paket->kitaplar;
    uyeler = paket->uyeler;
    oduncler = paket->oduncler;
    return true;
}

QString MainWindow::varsayilanBinaryDosyaYolu() const {
    return QCoreApplication::applicationDirPath() + "/kubys_veriler.bin";
}

void MainWindow::csvKatalogYukle() {
    if (arkaPlanThread != nullptr) {
        QMessageBox::warning(this, "Islem Devam Ediyor", "Zaten calisan bir arka plan islemi var.");
        return;
    }

    const QString dosyaYolu = QFileDialog::getOpenFileName(
        this,
        "CSV Katalog Yukle",
        QString(),
        "CSV Dosyalari (*.csv *.txt)"
    );

    if (dosyaYolu.isEmpty()) {
        return;
    }

    arkaPlanThread = new QThread(this);
    arkaPlanIsci = new ArkaPlanIsci();

    arkaPlanIsci->moveToThread(arkaPlanThread);

    connect(arkaPlanThread, &QThread::started, arkaPlanIsci, [this, dosyaYolu]() {
        arkaPlanIsci->csvKatalogYukle(dosyaYolu);
    });

    connect(arkaPlanIsci, &ArkaPlanIsci::ilerlemeGuncellendi, this, [this](int yuzde) {
        if (ilerlemeCubugu) {
            ilerlemeCubugu->setValue(yuzde);
        }
    });

    connect(
        arkaPlanIsci,
        &ArkaPlanIsci::katalogYuklemeTamamlandi,
        this,
        &MainWindow::katalogYuklemeTamamlandi
    );

    connect(arkaPlanIsci, &ArkaPlanIsci::hataOlustu, this, &MainWindow::arkaPlanHatasi);
    connect(arkaPlanIsci, &ArkaPlanIsci::iptalEdildi, this, &MainWindow::arkaPlanIptalEdildi);

    connect(arkaPlanIsci, &ArkaPlanIsci::katalogYuklemeTamamlandi, arkaPlanThread, &QThread::quit);
    connect(arkaPlanIsci, &ArkaPlanIsci::hataOlustu, arkaPlanThread, &QThread::quit);
    connect(arkaPlanIsci, &ArkaPlanIsci::iptalEdildi, arkaPlanThread, &QThread::quit);

    connect(arkaPlanThread, &QThread::finished, arkaPlanIsci, &QObject::deleteLater);
    connect(arkaPlanThread, &QThread::finished, arkaPlanThread, &QObject::deleteLater);
    connect(arkaPlanThread, &QThread::finished, this, &MainWindow::arkaPlanBitti);

    arkaPlanBasladi("CSV katalog arka planda yukleniyor...");
    arkaPlanThread->start();
}

void MainWindow::gecikmeRaporuOlustur() {
    if (arkaPlanThread != nullptr) {
        QMessageBox::warning(this, "Islem Devam Ediyor", "Zaten calisan bir arka plan islemi var.");
        return;
    }

    arkaPlanThread = new QThread(this);
    arkaPlanIsci = new ArkaPlanIsci();

    arkaPlanIsci->moveToThread(arkaPlanThread);

    const OduncListesi oduncListesi = oduncVektoru();

    connect(arkaPlanThread, &QThread::started, arkaPlanIsci, [this, oduncListesi]() {
        arkaPlanIsci->gecikmeRaporuOlustur(oduncListesi);
    });

    connect(arkaPlanIsci, &ArkaPlanIsci::ilerlemeGuncellendi, this, [this](int yuzde) {
        if (ilerlemeCubugu) {
            ilerlemeCubugu->setValue(yuzde);
        }
    });

    connect(
        arkaPlanIsci,
        &ArkaPlanIsci::raporTamamlandi,
        this,
        &MainWindow::gecikmeRaporuTamamlandi
    );

    connect(arkaPlanIsci, &ArkaPlanIsci::hataOlustu, this, &MainWindow::arkaPlanHatasi);
    connect(arkaPlanIsci, &ArkaPlanIsci::iptalEdildi, this, &MainWindow::arkaPlanIptalEdildi);

    connect(arkaPlanIsci, &ArkaPlanIsci::raporTamamlandi, arkaPlanThread, &QThread::quit);
    connect(arkaPlanIsci, &ArkaPlanIsci::hataOlustu, arkaPlanThread, &QThread::quit);
    connect(arkaPlanIsci, &ArkaPlanIsci::iptalEdildi, arkaPlanThread, &QThread::quit);

    connect(arkaPlanThread, &QThread::finished, arkaPlanIsci, &QObject::deleteLater);
    connect(arkaPlanThread, &QThread::finished, arkaPlanThread, &QObject::deleteLater);
    connect(arkaPlanThread, &QThread::finished, this, &MainWindow::arkaPlanBitti);

    arkaPlanBasladi("Gecikme raporu arka planda olusturuluyor...");
    arkaPlanThread->start();
}

void MainWindow::arkaPlanIsleminiIptalEt() {
    if (arkaPlanIsci) {
        arkaPlanIsci->iptalEt();
        statusBar()->showMessage("Arka plan islemi iptal ediliyor...");
    }
}

void MainWindow::arkaPlanBasladi(const QString& mesaj) {
    if (ilerlemeCubugu) {
        ilerlemeCubugu->setValue(0);
        ilerlemeCubugu->setVisible(true);
    }

    if (islemIptalEylemi) {
        islemIptalEylemi->setEnabled(true);
    }

    statusBar()->showMessage(mesaj);
}

void MainWindow::arkaPlanBitti() {
    if (ilerlemeCubugu) {
        ilerlemeCubugu->setValue(0);
        ilerlemeCubugu->setVisible(false);
    }

    if (islemIptalEylemi) {
        islemIptalEylemi->setEnabled(false);
    }

    arkaPlanThread = nullptr;
    arkaPlanIsci = nullptr;

    durumBilgisiniYenile();
}

void MainWindow::katalogYuklemeTamamlandi(const KitapListesi& yeniKitaplar) {
    int eklenen = 0;
    int atlanan = 0;

    for (const auto& kitap : yeniKitaplar) {
        if (kitaplar.ekle(kitap.isbn, kitap)) {
            ++eklenen;
        } else {
            ++atlanan;
        }
    }

    verileriModellereAktar();

    QMessageBox::information(
        this,
        "CSV Yukleme Tamamlandi",
        QString("Eklenen kitap sayisi: %1\nAtlanan tekrarli kayit sayisi: %2")
            .arg(eklenen)
            .arg(atlanan)
    );

    statusBar()->showMessage("CSV katalog yukleme tamamlandi.", 3000);
}

void MainWindow::gecikmeRaporuTamamlandi(const QString& rapor) {
    const QString dosyaYolu = QFileDialog::getSaveFileName(
        this,
        "Gecikme Raporunu Kaydet",
        "gecikme_raporu.txt",
        "Metin Dosyalari (*.txt)"
    );

    if (!dosyaYolu.isEmpty()) {
        QFile dosya(dosyaYolu);

        if (dosya.open(QIODevice::WriteOnly | QIODevice::Text)) {
            dosya.write(rapor.toUtf8());
            dosya.close();

            QMessageBox::information(this, "Rapor Kaydedildi", "Gecikme raporu dosyaya kaydedildi.");
        } else {
            QMessageBox::warning(this, "Hata", "Rapor dosyaya yazilamadi.");
        }
    } else {
        QMessageBox::information(this, "Gecikme Raporu", rapor);
    }

    statusBar()->showMessage("Gecikme raporu tamamlandi.", 3000);
}

void MainWindow::arkaPlanHatasi(const QString& mesaj) {
    QMessageBox::warning(this, "Arka Plan Hatasi", mesaj);
    statusBar()->showMessage("Arka plan isleminde hata olustu.", 3000);
}

void MainWindow::arkaPlanIptalEdildi() {
    QMessageBox::information(this, "Iptal", "Arka plan islemi iptal edildi.");
    statusBar()->showMessage("Arka plan islemi iptal edildi.", 3000);
}

void MainWindow::sunucuyuBaslat() {
    bool tamam = false;

    const int port = QInputDialog::getInt(
        this,
        "Sunucuyu Baslat",
        "Port:",
        45454,
        1024,
        65535,
        1,
        &tamam
    );

    if (!tamam) {
        return;
    }

    if (kutuphaneSunucu->baslat(static_cast<quint16>(port))) {
        QMessageBox::information(this, "Sunucu", QString("Sunucu baslatildi.\nPort: %1").arg(port));
    }
}

void MainWindow::sunucuyuDurdur() {
    kutuphaneSunucu->durdur();
    QMessageBox::information(this, "Sunucu", "Sunucu durduruldu.");
}

void MainWindow::sunucuyaBaglan() {
    bool tamam = false;

    const QString host = QInputDialog::getText(
        this,
        "Sunucuya Baglan",
        "Host:",
        QLineEdit::Normal,
        "127.0.0.1",
        &tamam
    );

    if (!tamam || host.trimmed().isEmpty()) {
        return;
    }

    const int port = QInputDialog::getInt(
        this,
        "Sunucuya Baglan",
        "Port:",
        45454,
        1024,
        65535,
        1,
        &tamam
    );

    if (!tamam) {
        return;
    }

    agIstemci->baglan(host.trimmed(), static_cast<quint16>(port));
}

void MainWindow::sunucuBaglantisiniKes() {
    agIstemci->baglantiyiKes();
    QMessageBox::information(this, "Ag", "Sunucu baglantisi kapatildi.");
}

void MainWindow::agKimlikDogrula() {
    bool tamam = false;

    const QString kullanici = QInputDialog::getText(
        this,
        "Kimlik Dogrula",
        "Kullanici adi:",
        QLineEdit::Normal,
        "admin",
        &tamam
    );

    if (!tamam || kullanici.trimmed().isEmpty()) {
        return;
    }

    const QString sifre = QInputDialog::getText(
        this,
        "Kimlik Dogrula",
        "Sifre:",
        QLineEdit::Password,
        QString(),
        &tamam
    );

    if (!tamam) {
        return;
    }

    agIstemci->kimlikDogrula(kullanici.trimmed(), sifre);
}

void MainWindow::uzakKitapAra() {
    bool tamam = false;

    const QString aranan = QInputDialog::getText(
        this,
        "Uzak Kitap Ara",
        "Aranacak kelime:",
        QLineEdit::Normal,
        QString(),
        &tamam
    );

    if (!tamam) {
        return;
    }

    agIstemci->kitapAra(aranan.trimmed());
}

void MainWindow::uzakOduncAl() {
    bool tamam = false;

    const int uyeNo = QInputDialog::getInt(
        this,
        "Uzak Odunc Al",
        "Uye No:",
        2001,
        1,
        999999,
        1,
        &tamam
    );

    if (!tamam) {
        return;
    }

    const QString isbn = QInputDialog::getText(
        this,
        "Uzak Odunc Al",
        "ISBN:",
        QLineEdit::Normal,
        "978-975-1000-01-1",
        &tamam
    );

    if (!tamam || isbn.trimmed().isEmpty()) {
        return;
    }

    agIstemci->oduncAl(uyeNo, isbn.trimmed());
}

void MainWindow::uzakIadeEt() {
    bool tamam = false;

    const int kayitId = QInputDialog::getInt(
        this,
        "Uzak Iade Et",
        "Odunc Kayit ID:",
        1,
        1,
        999999,
        1,
        &tamam
    );

    if (!tamam) {
        return;
    }

    agIstemci->iadeEt(kayitId);
}

void MainWindow::agCevabiIsle(const QJsonObject& cevap) {
    const QString durum = cevap["durum"].toString();
    const QString mesaj = cevap["mesaj"].toString();

    if (cevap.contains("kitaplar") && cevap["kitaplar"].isArray()) {
        const QJsonArray kitaplarDizisi = cevap["kitaplar"].toArray();

        QString metin;
        metin += mesaj + "\n\n";
        metin += QString("Bulunan kitap sayisi: %1\n\n").arg(kitaplarDizisi.size());

        for (const QJsonValue& deger : kitaplarDizisi) {
            const QJsonObject kitap = deger.toObject();

            metin += QString("[%1] %2 - %3 (%4) | Kategori: %5 | Kopya: %6\n")
                .arg(kitap["isbn"].toString())
                .arg(kitap["baslik"].toString())
                .arg(kitap["yazar"].toString())
                .arg(kitap["yayin_yili"].toInt())
                .arg(kitap["kategori"].toString())
                .arg(kitap["kopya_sayisi"].toInt());
        }

        QMessageBox::information(this, "Ag Cevabi", metin);
        return;
    }

    if (cevap.contains("kayit") && cevap["kayit"].isObject()) {
        const QJsonObject kayit = cevap["kayit"].toObject();

        QString metin;
        metin += mesaj + "\n\n";
        metin += QString("Kayit ID: %1\n").arg(kayit["kayit_id"].toInt());
        metin += QString("Uye No: %1\n").arg(kayit["uye_no"].toInt());
        metin += QString("ISBN: %1\n").arg(kayit["isbn"].toString());
        metin += QString("Odunc Tarihi: %1\n").arg(kayit["odunc_tarihi"].toString());
        metin += QString("Iade Tarihi: %1\n").arg(kayit["iade_tarihi"].toString());
        metin += QString("Durum: %1\n").arg(kayit["durum"].toString());

        QMessageBox::information(this, "Ag Cevabi", metin);
        return;
    }

    if (durum == "hata") {
        QMessageBox::warning(this, "Ag Cevabi", mesaj);
    } else {
        QMessageBox::information(this, "Ag Cevabi", mesaj);
    }
}

void MainWindow::pencereAyarlariniYukle() {
    QSettings ayarlar("KUBYS", "KutuphaneYonetimSistemi");

    const QByteArray geometri = ayarlar.value("pencere/geometri").toByteArray();

    if (!geometri.isEmpty()) {
        restoreGeometry(geometri);
    }
}

void MainWindow::pencereAyarlariniKaydet() {
    QSettings ayarlar("KUBYS", "KutuphaneYonetimSistemi");
    ayarlar.setValue("pencere/geometri", saveGeometry());
}

void MainWindow::closeEvent(QCloseEvent* event) {
    if (agIstemci) {
        agIstemci->baglantiyiKes();
    }

    if (kutuphaneSunucu && kutuphaneSunucu->calisiyorMu()) {
        kutuphaneSunucu->durdur();
    }

    if (arkaPlanIsci) {
        arkaPlanIsci->iptalEt();
    }

    if (arkaPlanThread) {
        arkaPlanThread->quit();
        arkaPlanThread->wait(3000);
    }

    binaryDosyasinaYaz(varsayilanBinaryDosyaYolu());
    pencereAyarlariniKaydet();
    QMainWindow::closeEvent(event);
}