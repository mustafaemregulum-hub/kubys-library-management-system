#pragma once

#include "../include/varliklar.h"

#include <QDialog>

class QDateEdit;
class QLineEdit;
class QSpinBox;

class UyeDialog : public QDialog {
    Q_OBJECT

public:
    explicit UyeDialog(QWidget* parent = nullptr);

    Uye uye() const;

protected:
    void accept() override;

private:
    QSpinBox* uyeNoSpin{};
    QLineEdit* isimEdit{};
    QLineEdit* soyisimEdit{};
    QLineEdit* telefonEdit{};
    QDateEdit* tarihEdit{};

    void formuKur();
    bool dogrula();
};