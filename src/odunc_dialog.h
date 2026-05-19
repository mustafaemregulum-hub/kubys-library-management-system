#pragma once

#include "../include/varliklar.h"

#include <QDialog>
#include <vector>

class QComboBox;
class QDateEdit;

class OduncDialog : public QDialog {
    Q_OBJECT

public:
    OduncDialog(
        const std::vector<Uye>& uyeler,
        const std::vector<Kitap>& kitaplar,
        int yeniKayitId,
        QWidget* parent = nullptr
    );

    OduncKaydi kayit() const;

protected:
    void accept() override;

private:
    std::vector<Uye> mevcutUyeler;
    std::vector<Kitap> mevcutKitaplar;
    int kayitId{};

    QComboBox* uyeCombo{};
    QComboBox* kitapCombo{};
    QDateEdit* tarihEdit{};

    void formuKur();
    bool dogrula();
};