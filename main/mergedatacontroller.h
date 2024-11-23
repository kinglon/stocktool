#ifndef MERGERDATACONTROLLER_H
#define MERGERDATACONTROLLER_H

#include <QObject>
#include <QDate>
#include <QThread>
#include "datamanager.h"

class DayLineData
{
public:
    // 日数据
    StockData m_dayStockData;

    // 时间
    qint64 m_dayTime = 0;

    // 涨幅
    QString m_zhangfu;
};

class DataMerger : public QObject
{
    Q_OBJECT
public:
    explicit DataMerger(QObject *parent = nullptr);

public:
    void run();

signals:
    // 完成一支股票后发送
    void oneFinish();

    void runFinish(DataMerger* dataMerger);

protected:
    void doMerge();

    bool loadData();

    bool parseDayLine(const QString& line, DayLineData& dayLineData);

    void mergeDayAndDayLine();

    StockData getMonthDataByDayData(const StockData& stockData);

    virtual QString getCompareResult() = 0;

    void appendSpaceChar(QString& result, int count);

    void save(const QString& result);

public:    
    QVector<QString> m_stockPaths;

    QVector<QString> m_industryNames;

    bool m_compare2Part = true;

    int m_currentIndex = 0;

    QVector<StockData> m_dayStockData;

    QVector<StockData> m_monthStockData;

    QVector<StockData> m_yearStockData;

    QVector<DayLineData> m_dayLineDatas;
};

class MergeDataController : public QObject
{
    Q_OBJECT
public:
    explicit MergeDataController(QObject *parent = nullptr);

public:
    // compare2Part True 2宫比对，False 4宫比对
    void run(QString rootDir, bool compare2Part);

    static bool removeDir(const QString &dirName);

protected:
    virtual DataMerger* getDataMerger() = 0;

signals:
    void printLog(QString content);

    void runFinish();

private slots:
    // 处理完一支股票
    void onMergeOneFinish();

    void onMergerRunFinish(DataMerger* dataMerger);

private:
    void scanStocks(QString rootDir);

private:
    QVector<DataMerger*> m_dataMergers;

    QVector<QThread*> m_dataMergerThreads;

    QVector<QString> m_stockPaths;

    QVector<QString> m_industryNames;

    int m_finishStocks = 0;
};

#endif // MERGERDATACONTROLLER_H
