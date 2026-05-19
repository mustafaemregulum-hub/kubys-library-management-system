#pragma once

#include "../include/varliklar.h"

#include <QDialog>

class QLineEdit;
class QSpinBox;

class KitapDialog : public QDialog {
    Q_OBJECT

public:
    explicit KitapDialog(QWidget* parent = nullptr);

    Kitap kitap() const;

protected:
    void accept() override;

private:
    QLineEdit* isbnEdit{};
    QLineEdit* baslikEdit{};
    QLineEdit* yazarEdit{};
    QSpinBox* yilSpin{};
    QLineEdit* kategoriEdit{};
    QSpinBox* kopyaSpin{};

    void formuKur();
    bool dogrula();
};