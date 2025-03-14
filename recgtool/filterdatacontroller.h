#ifndef FILTERDATACONTROLLER_H
#define FILTERDATACONTROLLER_H

#include <QObject>
#include "stockdatamanager.h"
#include <QDate>

class DataFilterBase : public QObject
{
    Q_OBJECT
public:
    explicit DataFilterBase(QObject *parent = nullptr);

public:
    void run();

    // 查找公历开始时间小于beginTime附近的索引
    static int findIndex(const QVector<StockData>& stockDatas, qint64 beginSearchTime);

    // 查找公历开始时间小于beginTime附近的索引
    static int findIndex(const QVector<DayLineData>& dayLineDatas, qint64 beginSearchTime);

protected:
    virtual bool canMatch(const StockData& stockData) = 0;

private:
    bool isZhangFuOk(const StockData& stockData, int stockType);

signals:
    void oneDayFinish();

    void runFinish(DataFilterBase* dataFilter);

public:
    QDate m_beginDate;

    // 不含这天
    QDate m_endDate;

    QVector<StockData> m_stockDatas;
};

// 精确查找
class DataFilterJingQue : public DataFilterBase
{
    Q_OBJECT

public:
    explicit DataFilterJingQue(QObject *parent = nullptr);

protected:
    virtual bool canMatch(const StockData& stockData) override;
};

// 含关键字查找
class DataFilterKeyWord : public DataFilterBase
{
    Q_OBJECT

public:
    explicit DataFilterKeyWord(QObject *parent = nullptr);

protected:
    virtual bool canMatch(const StockData& stockData) override;
};

// 算法查找
class DataFilterAlgorithm : public DataFilterBase
{
    Q_OBJECT

public:
    explicit DataFilterAlgorithm(QObject *parent = nullptr);

protected:
    virtual bool canMatch(const StockData& stockData) override;
};

class FilterDataController : public QObject
{
    Q_OBJECT
public:
    explicit FilterDataController(QObject *parent = nullptr);

public:
    void run();

signals:
    void printLog(QString content);

    void runFinish();

private slots:
    void onFilterOneDayFinish();

    void onFilterRunFinish(DataFilterBase* dataFilter);

public:
    // 识别成功的数据
    QVector<StockData> m_stockDatas;

private:
    QVector<DataFilterBase*> m_dataFilters;

    QVector<QThread*> m_dataFilterThreads;

    int m_finishDays = 0;

    int m_totalDays = 0;
};

#endif // FILTERDATACONTROLLER_H
