#include "uye_model.h"

UyeModel::UyeModel(QObject* parent)
    : QAbstractTableModel(parent)
{
}

void UyeModel::uyeleriAyarla(const std::vector<Uye>& yeniUyeler) {
    beginResetModel();
    uyeler = yeniUyeler;
    endResetModel();
}

int UyeModel::rowCount(const QModelIndex& parent) const {
    if (parent.isValid()) {
        return 0;
    }

    return static_cast<int>(uyeler.size());
}

int UyeModel::columnCount(const QModelIndex& parent) const {
    if (parent.isValid()) {
        return 0;
    }

    return 5;
}

QVariant UyeModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid()) {
        return {};
    }

    if (index.row() < 0 || index.row() >= static_cast<int>(uyeler.size())) {
        return {};
    }

    const Uye& uye = uyeler.at(static_cast<std::size_t>(index.row()));

    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        switch (index.column()) {
            case 0: return uye.uye_no;
            case 1: return QString::fromStdString(uye.isim);
            case 2: return QString::fromStdString(uye.soyisim);
            case 3: return QString::fromStdString(uye.telefon);
            case 4: return QString::fromStdString(uye.kayit_tarihi);
            default: return {};
        }
    }

    return {};
}

QVariant UyeModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (role != Qt::DisplayRole || orientation != Qt::Horizontal) {
        return {};
    }

    switch (section) {
        case 0: return "Uye No";
        case 1: return "Isim";
        case 2: return "Soyisim";
        case 3: return "Telefon";
        case 4: return "Kayit Tarihi";
        default: return {};
    }
}