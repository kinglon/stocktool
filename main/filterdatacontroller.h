#ifndef FILTERDATACONTROLLER_H
#define FILTERDATACONTROLLER_H

#include <QObject>
#include <QDate>
#include <QThread>
#include "../stock/datamanager.h"
#include "../stock/stockdatautil.h"

class FilterParam
{
public:
    // 标志是否为无损过滤
    bool m_notLossFilter = false;

    // 无损过滤时有效，选择筛选的数据
    int m_notLossFilterDataType = STOCK_DATA_YEAR;

    // 有损过滤时有效，只过滤时的数据
    int m_onlyFilterHour = false;

    // 有损过滤时有效，只过滤到月的数据
    int m_onlyFilterToMonth = false;

    // 有损过滤时有效，标志是否全宫相关
    bool m_matchAll = false;

    // 过滤时间范围
    QDate m_beginDate;
    QDate m_endDate; // 不含

public:
    bool isOnlyFilterHourData()
    {
        return !m_notLossFilter && m_onlyFilterHour;
    }

    bool isStockDataYearOrMonth()
    {
        if (m_notLossFilter &&
                (m_notLossFilterDataType == STOCK_DATA_YEAR || m_notLossFilterDataType == STOCK_DATA_MONTH))
        {
            return true;
        }
        else if (!m_notLossFilter && m_onlyFilterToMonth)
        {
            return true;
        }

        return false;
    }
};

class DataFilter : public QObject
{
    Q_OBJECT
public:
    explicit DataFilter(QObject *parent = nullptr);

public:
    void run();

    // 查找公历开始时间小于beginTime附近的索引
    static int findIndex(const QVector<StockData>& stockDatas, qint64 beginSearchTime);

signals:
    void oneDayFinish();

    void runFinish(DataFilter* dataFilter);

private:
    void filterByNotLoss(QDate date);

    void filterOnlyToMonth(QDate date);

    void filterNotOnlyToMonth(QDate date);

    void filterOnlyHourData(QDate date);

    void filterYearData(QDate date, bool notLossFilter, bool useLunarTime, QVector<StockData>& yearStockDatas);

    void filterMonthData(QDate date, bool notLossFilter, bool useLunarTime, const QVector<StockData>& yearStockDatas, QVector<StockData>& monthStockDatas);

    void filterDayData(QDate date, bool notLossFilter, const QVector<StockData>& monthStockDatas, QVector<StockData>& dayStockDatas);

    void filterSecondDayData(QDate date, const QVector<StockData>& dayStockDatas, QVector<StockData>& secondDayStockDatas);

    void filterHourData(QDate date, const QVector<StockData>& dayStockDatas, int dataType, int filterType, QVector<StockData>& hourStockDatas);

    void filterHourData(QDate date, bool notLossFilter, int dataType, int filterType, QVector<StockData>& hourStockDatas);

public:
    bool m_notLossFilter = false;

    // 无损过滤使用的数据
    int m_notLossFilterDataType = STOCK_DATA_YEAR;

    bool m_onlyFilterHour = false;

    bool m_onlyFilterToMonth = true;

    bool m_matchAll = true;

    QDate m_beginDate;

    QDate m_endDate;

    QVector<StockData> m_stockDatas;
};

class FilterDataController : public QObject
{
    Q_OBJECT
public:
    explicit FilterDataController(QObject *parent = nullptr);

public:
    void run(const FilterParam& filterParam);

signals:
    void printLog(QString content);

    void runFinish();

private slots:
    void onFilterOneDayFinish();

    void onFilterRunFinish(DataFilter* dataFilter);

private:
    void saveStockData();

    void saveStockDataDetail();

    void saveStockDataDetail(int begin, int end);

    void saveStockHourDataSummaryInfo();

    void appendSpaceChar(QString& result, int count);

    StockData findYearStockDataByMonth(const StockData& monthStockData);

    StockData findMonthStockDataByDay(const StockData& dayStockData);

    StockData findLastDayStockDataByDay(const StockData& dayStockData);

    void appendDayStockData(int prefixSpaceCount, QString& result, const StockData& stockData);

public:
    // 过滤名字，不能为空
    QString m_name;

private:
    FilterParam m_filterParam;

    QVector<DataFilter*> m_dataFilters;

    QVector<QThread*> m_dataFilterThreads;

    QVector<StockData> m_stockDatas;

    int m_finishDays = 0;

    int m_totalDays = 0;
};

#endif // FILTERDATACONTROLLER_H
