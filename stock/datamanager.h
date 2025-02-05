#ifndef DATAMANAGER_H
#define DATAMANAGER_H

#include <QVector>
#include "stockdatautil.h"

class DataManager
{
protected:
    DataManager();

public:
    static DataManager* getInstance();

    bool hasData();

    void clearData();

    int totalCount();

    // 按公历时间排序
    void sort();

public:
    QVector<StockData> m_stockDatas[MAX_STOCK_DATA_COUNT];
};

#endif // DATAMANAGER_H
