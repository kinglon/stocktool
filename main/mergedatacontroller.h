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

    // 月数据
    StockData m_monthStockData;

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

private:
    void doMerge();

    bool loadData();

    bool parseDayLine(const QString& line, DayLineData& dayLineData);

    void mergeDayAndDayLine();

    void filterData(QVector<DayLineData>& dayLineDatas);

    void mergeMonthData(QVector<DayLineData>& dayLineDatas);

    void save(QVector<DayLineData>& dayLineDatas);

public:    
    QVector<QString> m_stockPaths;

    QVector<QString> m_industryNames;

    bool m_compare2Part = true;

    int m_currentIndex = 0;

    QVector<StockData> m_dayStockData;

    QVector<StockData> m_monthStockData;

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
