#ifndef DAYMERGEDATACONTROLLER_H
#define DAYMERGEDATACONTROLLER_H

#include "mergedatacontroller.h"

class DayDataMerger : public DataMerger
{
    Q_OBJECT
public:
    explicit DayDataMerger(QObject *parent = nullptr);

protected:
    virtual QString getCompareResult() override;

private:
    void filterData(const StockData& stockData, QVector<DayLineData>& dayLineDatas);

    void appendData(const StockData& stockData, const QVector<DayLineData>& dayLineDatas, QString& result);

public:
    qint64 m_beginDate = 0;

    qint64 m_endDate = 0;
};

// 按日比对
class DayMergeDataController : public MergeDataController
{
    Q_OBJECT
public:
    explicit DayMergeDataController(QObject *parent = nullptr);

protected:
    virtual DataMerger* getDataMerger() override;

public:
    qint64 m_beginDate = 0;

    qint64 m_endDate = 0;
};

#endif // DAYMERGEDATACONTROLLER_H
