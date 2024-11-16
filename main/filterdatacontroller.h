#ifndef FILTERDATACONTROLLER_H
#define FILTERDATACONTROLLER_H

#include <QObject>
#include <QDate>
#include <QThread>
#include "datamanager.h"

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
    void filterOnlyToMonth(QDate date);

    void filterNotOnlyToMonth(QDate date);

    void filterYearData(QDate date, bool useLunarTime, QVector<StockData>& yearStockDatas);

    void filterMonthData(QDate date, bool useLunarTime, const QVector<StockData>& yearStockDatas, QVector<StockData>& monthStockDatas);

    void filterDayData(QDate date, const QVector<StockData>& monthStockDatas, QVector<StockData>& dayStockDatas);

    void filterSecondDayData(QDate date, const QVector<StockData>& dayStockDatas, QVector<StockData>& secondDayStockDatas);

    void filterHourData(QDate date, const QVector<StockData>& dayStockDatas, int dataType, int filterType, QVector<StockData>& hourStockDatas);

    // 检查是否满足条件
    bool checkIfStockDataOk(StockData stockData, const FilterCondition& filterCondition);

    // 按规则检查是否有存字
    bool hasCunWord(QString data, QString data1, QString data2);

    // 检查4个宫格是否有不带括号的字
    bool haveWordWithoutKuohao(QString word, QString data[4]);    

public:
    bool m_onlyFilterToMonth = true;

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
    void run(bool onlyFilterToMonth, QDate beginDate, QDate endDate);

signals:
    void printLog(QString content);

    void runFinish();

private slots:
    void onFilterOneDayFinish();

    void onFilterRunFinish(DataFilter* dataFilter);

private:
    void saveStockData();

    void saveStockDataDetail();

    void appendSpaceChar(QString& result, int count);

    StockData findYearStockDataByMonth(const StockData& monthStockData);

    StockData findMonthStockDataByDay(const StockData& dayStockData);

    StockData findLastDayStockDataByDay(const StockData& dayStockData);

    void appendDayStockData(int prefixSpaceCount, QString& result, const StockData& stockData);

private:
    bool m_onlyFilterToMonth = false;

    QVector<DataFilter*> m_dataFilters;

    QVector<QThread*> m_dataFilterThreads;

    QVector<StockData> m_stockDatas;

    int m_finishDays = 0;

    int m_totalDays = 0;
};

#endif // FILTERDATACONTROLLER_H
