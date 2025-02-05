#ifndef SCORECONTROLLER_H
#define SCORECONTROLLER_H

#include <QObject>
#include <QDate>
#include <QThread>
#include "../stock/datamanager.h"
#include "../stock/stockdatautil.h"
#include "settingmanager.h"

class ScoreParam
{
public:
    // 控制器名字
    QString m_name;

    // 标志是否取二宫最大值，如果不是就取二宫之和再乘以系数
    bool m_isGetMax = true;

    // 取和后的系数，百分比
    int m_sumFactor = 50;

    // 标志是否升序排序
    bool m_orderUp = true;

    // 打分的日期范围
    QDate m_beginDate;
    QDate m_endDate; // 不含

    // 打分的数据类型
    int m_dataType = STOCK_DATA_DAY;
};

class ScoreResult
{
public:
    // 股票名称
    QString m_stockName;

    // 股票数据公历时间的时间戳
    qint64 m_beginTime = 0;

    // 对应打分12项
    bool m_hasWords[SCORE_COUNT] = {false};

public:
    // 判断是否为0分
    bool isZero()
    {
        for (bool hasWord : m_hasWords)
        {
            if (hasWord)
            {
                return false;
            }
        }

        return true;
    }
};

class StockScore
{
public:
    QString m_stockName;

    int m_score = 0;
};

class DataScorer : public QObject
{
    Q_OBJECT
public:
    explicit DataScorer(QObject *parent = nullptr);

public:
    void run();

    // 查找公历开始时间小于beginTime附近的索引
    static int findIndex(const QVector<StockData>& stockDatas, qint64 beginSearchTime);

signals:
    void oneDayFinish();

    void runFinish(DataScorer* dataFilter);

private:
    void score(QDate date);

    QVector<StockData> getStockDatasByDate(const QDate& date);

    QVector<StockData> getYearStockDatasByDate(const QDate& date);

    QVector<StockData> getMonthStockDatasByDate(const QDate& date);

    QVector<StockData> getDayStockDatasByDate(const QDate& date);

public:
    // 无损过滤使用的数据
    int m_scoreDataType = STOCK_DATA_YEAR;

    QDate m_beginDate;

    QDate m_endDate;

    QVector<ScoreResult> m_scoreResults;
};

class ScoreDataController : public QObject
{
    Q_OBJECT
public:
    explicit ScoreDataController(QObject *parent = nullptr);

public:
    void run(const ScoreParam& scoreParam);

signals:
    void printLog(QString content);

    void runFinish();

private slots:
    void onScoreOneDayFinish();

    void onScoreRunFinish(DataScorer* dataFilter);

private:
    void summaryScore();

    int getStockScore(const ScoreResult& stockResult);

    QString getDateRangeString();

private:
    ScoreParam m_scoreParam;

    QVector<DataScorer*> m_dataScorers;

    QVector<QThread*> m_dataScorerThreads;

    QVector<ScoreResult> m_scoreResults;

    int m_finishDays = 0;

    int m_totalDays = 0;

public:
    QString m_resultString;
};

#endif // SCORECONTROLLER_H
