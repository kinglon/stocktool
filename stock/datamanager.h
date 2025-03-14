#ifndef DATAMANAGER_H
#define DATAMANAGER_H

#include <QVector>
#include <QMap>
#include "stockdatautil.h"

class DataManager
{
protected:
    DataManager();

public:
    static DataManager* getInstance();

    static DataManager* createInstance() { return new DataManager(); }

    bool hasData();

    void clearData();

    int totalCount();

    // 按公历时间排序
    void sort();

    // 获取限数据，返回数据按时间排序
    QVector<StockData> getXianDatas(const QString& industry, const QString& stockName);

public:
    QVector<StockData> m_stockDatas[MAX_STOCK_DATA_COUNT];

    QVector<DayLineData> m_dayLineDatas[MAX_STOCK_DATA_COUNT];
};

#endif // DATAMANAGER_H
