#ifndef STOCKDATAMANAGER_H
#define STOCKDATAMANAGER_H

#include <QString>
#include <QVector>
#include "../stock/datamanager.h"
#include "settingmanager.h"

class StockDataManager
{
protected:
    StockDataManager();

public:
    static StockDataManager* getInstance();

    // 判断有没结果数据
    bool hasResultData() { return !m_result.isEmpty(); }

    void clear();

public:
    QString m_result;

    DataManager* m_dataManagers[STOCK_TYPE_MAX];
};

#endif // STOCKDATAMANAGER_H
