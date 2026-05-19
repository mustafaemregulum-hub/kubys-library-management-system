#include "odunc_dialog.h"

#include <QComboBox>
#include <QDate>
#include <QDateEdit>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QMessageBox>
#include <QVBoxLayout>

OduncDialog::OduncDialog(
    const std::vector<Uye>& uyeler,
    const std::vector<Kitap>& kitaplar,
    int yeniKayitId,
    QWidget* parent
)
    : QDialog(parent),
      mevcutUyeler(uyeler),
      mevcutKitaplar(kitaplar),
      kayitId(yeniKayitId)
{
    formuKur();
}

void OduncDialog::formuKur() {
    setWindowTitle("Odunc Ver");

    uyeCombo = new QComboBox(this);
    kitapCombo = new QComboBox(this);
    tarihEdit = new QDateEdit(this);

    for (const auto& uye : mevcutUyeler) {
        const QString metin = QString("%1 - %2 %3")
            .arg(uye.uye_no)
            .arg(QString::fromStdString(uye.isim))
            .arg(QString::fromStdString(uye.soyisim));

        uyeCombo->addItem(metin, uye.uye_no);
    }

    for (const auto& kitap : mevcutKitaplar) {
        const QString metin = QString("%1 - %2 | Kopya: %3")
            .arg(QString::fromStdString(kitap.isbn))
            .arg(QString::fromStdString(kitap.baslik))
            .arg(kitap.kopya_sayisi);

        kitapCombo->addItem(metin, QString::fromStdString(kitap.isbn));
    }

    tarihEdit->setCalendarPopup(true);
    tarihEdit->setDate(QDate::currentDate());
    tarihEdit->setDisplayFormat("yyyy-MM-dd");

    QFormLayout* form = new QFormLayout();
    form->addRow("Uye:", uyeCombo);
    form->addRow("Kitap:", kitapCombo);
    form->addRow("Odunc Tarihi:", tarihEdit);

    QDialogButtonBox* butonlar = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
        this
    );

    connect(butonlar, &QDialogButtonBox::accepted, this, &OduncDialog::accept);
    connect(butonlar, &QDialogButtonBox::rejected, this, &OduncDialog::reject);

    QVBoxLayout* anaYerlesim = new QVBoxLayout(this);
    anaYerlesim->addLayout(form);
    anaYerlesim->addWidget(butonlar);
}

bool OduncDialog::dogrula() {
    if (uyeCombo->count() == 0) {
        QMessageBox::warning(this, "Uye Yok", "Odunc islemi icin once uye eklenmelidir.");
        return false;
    }

    if (kitapCombo->count() == 0) {
        QMessageBox::warning(this, "Kitap Yok", "Odunc islemi icin once kitap eklenmelidir.");
        return false;
    }

    return true;
}

void OduncDialog::accept() {
    if (!dogrula()) {
        return;
    }

    QDialog::accept();
}

OduncKaydi OduncDialog::kayit() const {
    return {
        kayitId,
        uyeCombo->currentData().toInt(),
        kitapCombo->currentData().toString().toStdString(),
        tarihEdit->date().toString("yyyy-MM-dd").toStdString(),
        std::nullopt,
        OduncDurum::Oduncte
    };
}