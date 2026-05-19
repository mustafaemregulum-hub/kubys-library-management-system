#include "kitap_dialog.h"

#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLineEdit>
#include <QMessageBox>
#include <QRegularExpression>
#include <QSpinBox>
#include <QVBoxLayout>

KitapDialog::KitapDialog(QWidget* parent)
    : QDialog(parent)
{
    formuKur();
}

void KitapDialog::formuKur() {
    setWindowTitle("Kitap Ekle");

    isbnEdit = new QLineEdit(this);
    baslikEdit = new QLineEdit(this);
    yazarEdit = new QLineEdit(this);
    yilSpin = new QSpinBox(this);
    kategoriEdit = new QLineEdit(this);
    kopyaSpin = new QSpinBox(this);

    yilSpin->setRange(1500, 2100);
    yilSpin->setValue(2025);

    kopyaSpin->setRange(1, 1000);
    kopyaSpin->setValue(1);

    QFormLayout* form = new QFormLayout();
    form->addRow("ISBN:", isbnEdit);
    form->addRow("Baslik:", baslikEdit);
    form->addRow("Yazar:", yazarEdit);
    form->addRow("Yayin Yili:", yilSpin);
    form->addRow("Kategori:", kategoriEdit);
    form->addRow("Kopya Sayisi:", kopyaSpin);

    QDialogButtonBox* butonlar = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
        this
    );

    connect(butonlar, &QDialogButtonBox::accepted, this, &KitapDialog::accept);
    connect(butonlar, &QDialogButtonBox::rejected, this, &KitapDialog::reject);

    QVBoxLayout* anaYerlesim = new QVBoxLayout(this);
    anaYerlesim->addLayout(form);
    anaYerlesim->addWidget(butonlar);
}

bool KitapDialog::dogrula() {
    if (isbnEdit->text().trimmed().isEmpty() ||
        baslikEdit->text().trimmed().isEmpty() ||
        yazarEdit->text().trimmed().isEmpty() ||
        kategoriEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "Eksik Bilgi", "Lutfen tum alanlari doldurun.");
        return false;
    }

    const QRegularExpression isbnRegex("^[0-9-]{10,20}$");
    if (!isbnRegex.match(isbnEdit->text().trimmed()).hasMatch()) {
        QMessageBox::warning(this, "Gecersiz ISBN", "ISBN sadece rakam ve tire icermelidir.");
        return false;
    }

    return true;
}

void KitapDialog::accept() {
    if (!dogrula()) {
        return;
    }

    QDialog::accept();
}

Kitap KitapDialog::kitap() const {
    return {
        isbnEdit->text().trimmed().toStdString(),
        baslikEdit->text().trimmed().toStdString(),
        yazarEdit->text().trimmed().toStdString(),
        yilSpin->value(),
        kategoriEdit->text().trimmed().toStdString(),
        kopyaSpin->value()
    };
}