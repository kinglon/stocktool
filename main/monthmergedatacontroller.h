#ifndef MONTHMERGEDATACONTROLLER_H
#define MONTHMERGEDATACONTROLLER_H

#include "mergedatacontroller.h"
#include <QMap>

class MonthDataMerger : public DataMerger
{
    Q_OBJECT
public:
    explicit MonthDataMerger(QObject *parent = nullptr);

protected:
    virtual QString getCompareResult() override;

private:
    void filterData(const StockData& stockData, QVector<DayLineData>& dayLineDatas);

public:
    QMap<QString, QVector<QString>>* m_stockName2MonthList = nullptr;
};

// 按月比对
class MonthMergeDataController : public MergeDataController
{
    Q_OBJECT
public:
    explicit MonthMergeDataController(QObject *parent = nullptr);

protected:
    virtual DataMerger* getDataMerger() override;

private:
    void loadResultData();

private:
    // 每支股票筛查的月，如：贵州茅台, [2023年1月，2023年6月]
    QMap<QString, QVector<QString>> m_stockName2MonthList;

    // 标志是否加载数据, m_stockName2MonthList
    bool m_hasLoaded = false;
};

#endif // MONTHMERGEDATACONTROLLER_H
