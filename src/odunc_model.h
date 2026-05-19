#pragma once

#include "../include/varliklar.h"

#include <QAbstractTableModel>
#include <vector>

class OduncModel : public QAbstractTableModel {
    Q_OBJECT

public:
    explicit OduncModel(QObject* parent = nullptr);

    void oduncleriAyarla(const std::vector<OduncKaydi>& yeniOduncler);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

private:
    std::vector<OduncKaydi> oduncler;
};