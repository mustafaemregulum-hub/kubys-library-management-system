#include "odunc_model.h"

#include <QBrush>
#include <QColor>

OduncModel::OduncModel(QObject* parent)
    : QAbstractTableModel(parent)
{
}

void OduncModel::oduncleriAyarla(const std::vector<OduncKaydi>& yeniOduncler) {
    beginResetModel();
    oduncler = yeniOduncler;
    endResetModel();
}

int OduncModel::rowCount(const QModelIndex& parent) const {
    if (parent.isValid()) {
        return 0;
    }

    return static_cast<int>(oduncler.size());
}

int OduncModel::columnCount(const QModelIndex& parent) const {
    if (parent.isValid()) {
        return 0;
    }

    return 6;
}

QVariant OduncModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid()) {
        return {};
    }

    if (index.row() < 0 || index.row() >= static_cast<int>(oduncler.size())) {
        return {};
    }

    const OduncKaydi& kayit = oduncler.at(static_cast<std::size_t>(index.row()));

    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        switch (index.column()) {
            case 0: return kayit.kayit_id;
            case 1: return kayit.uye_no;
            case 2: return QString::fromStdString(kayit.isbn);
            case 3: return QString::fromStdString(kayit.odunc_tarihi);
            case 4: return QString::fromStdString(kayit.iade_tarihi.value_or("---"));
            case 5: return QString::fromStdString(durumYazisi(kayit.durum));
            default: return {};
        }
    }

    if (role == Qt::BackgroundRole && kayit.durum == OduncDurum::Gecikmis) {
        return QBrush(QColor(255, 220, 220));
    }

    return {};
}

QVariant OduncModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (role != Qt::DisplayRole || orientation != Qt::Horizontal) {
        return {};
    }

    switch (section) {
        case 0: return "Kayit ID";
        case 1: return "Uye No";
        case 2: return "ISBN";
        case 3: return "Odunc Tarihi";
        case 4: return "Iade Tarihi";
        case 5: return "Durum";
        default: return {};
    }
}