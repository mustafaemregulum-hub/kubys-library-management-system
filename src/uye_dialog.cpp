#include "uye_dialog.h"

#include <QDate>
#include <QDateEdit>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLineEdit>
#include <QMessageBox>
#include <QSpinBox>
#include <QVBoxLayout>

UyeDialog::UyeDialog(QWidget* parent)
    : QDialog(parent)
{
    formuKur();
}

void UyeDialog::formuKur() {
    setWindowTitle("Uye Ekle");

    uyeNoSpin = new QSpinBox(this);
    isimEdit = new QLineEdit(this);
    soyisimEdit = new QLineEdit(this);
    telefonEdit = new QLineEdit(this);
    tarihEdit = new QDateEdit(this);

    uyeNoSpin->setRange(1, 999999);
    uyeNoSpin->setValue(3001);

    tarihEdit->setCalendarPopup(true);
    tarihEdit->setDate(QDate::currentDate());
    tarihEdit->setDisplayFormat("yyyy-MM-dd");

    QFormLayout* form = new QFormLayout();
    form->addRow("Uye No:", uyeNoSpin);
    form->addRow("Isim:", isimEdit);
    form->addRow("Soyisim:", soyisimEdit);
    form->addRow("Telefon:", telefonEdit);
    form->addRow("Kayit Tarihi:", tarihEdit);

    QDialogButtonBox* butonlar = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
        this
    );

    connect(butonlar, &QDialogButtonBox::accepted, this, &UyeDialog::accept);
    connect(butonlar, &QDialogButtonBox::rejected, this, &UyeDialog::reject);

    QVBoxLayout* anaYerlesim = new QVBoxLayout(this);
    anaYerlesim->addLayout(form);
    anaYerlesim->addWidget(butonlar);
}

bool UyeDialog::dogrula() {
    if (isimEdit->text().trimmed().isEmpty() ||
        soyisimEdit->text().trimmed().isEmpty() ||
        telefonEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "Eksik Bilgi", "Lutfen tum alanlari doldurun.");
        return false;
    }

    return true;
}

void UyeDialog::accept() {
    if (!dogrula()) {
        return;
    }

    QDialog::accept();
}

Uye UyeDialog::uye() const {
    return {
        uyeNoSpin->value(),
        isimEdit->text().trimmed().toStdString(),
        soyisimEdit->text().trimmed().toStdString(),
        telefonEdit->text().trimmed().toStdString(),
        tarihEdit->date().toString("yyyy-MM-dd").toStdString()
    };
}