#ifndef DATAMANAGER_H
#define DATAMANAGER_H

#include <QVector>
#include "datamodel.h"

// 股票数据索引
#define STOCK_DATA_YEAR         0
#define STOCK_DATA_MONTH        1
#define STOCK_DATA_DAY          2
#define STOCK_DATA_HOUR         3
#define STOCK_DATA_HOUR_YI      3
#define STOCK_DATA_HOUR_WU      4
#define STOCK_DATA_HOUR_WEI     5
#define MAX_STOCK_DATA_COUNT    6

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
