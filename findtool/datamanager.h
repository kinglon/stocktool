#pragma once

#include <QString>
#include <QVector>
#include "datamodel.h"

class DataManager
{
protected:
    DataManager();

public:
    static DataManager* getInstance();

public:
    // 股票
    QVector<Stock> m_stocks;
};
