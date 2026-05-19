#include "kitap_model.h"

KitapModel::KitapModel(QObject* parent)
    : QAbstractTableModel(parent)
{
}

void KitapModel::kitaplariAyarla(const std::vector<Kitap>& yeniKitaplar) {
    beginResetModel();
    kitaplar = yeniKitaplar;
    endResetModel();
}

int KitapModel::rowCount(const QModelIndex& parent) const {
    if (parent.isValid()) {
        return 0;
    }

    return static_cast<int>(kitaplar.size());
}

int KitapModel::columnCount(const QModelIndex& parent) const {
    if (parent.isValid()) {
        return 0;
    }

    return 6;
}

QVariant KitapModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid()) {
        return {};
    }

    if (index.row() < 0 || index.row() >= static_cast<int>(kitaplar.size())) {
        return {};
    }

    const Kitap& kitap = kitaplar.at(static_cast<std::size_t>(index.row()));

    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        switch (index.column()) {
            case 0: return QString::fromStdString(kitap.isbn);
            case 1: return QString::fromStdString(kitap.baslik);
            case 2: return QString::fromStdString(kitap.yazar);
            case 3: return kitap.yayin_yili;
            case 4: return QString::fromStdString(kitap.kategori);
            case 5: return kitap.kopya_sayisi;
            default: return {};
        }
    }

    return {};
}

QVariant KitapModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (role != Qt::DisplayRole || orientation != Qt::Horizontal) {
        return {};
    }

    switch (section) {
        case 0: return "ISBN";
        case 1: return "Baslik";
        case 2: return "Yazar";
        case 3: return "Yayin Yili";
        case 4: return "Kategori";
        case 5: return "Kopya Sayisi";
        default: return {};
    }
}