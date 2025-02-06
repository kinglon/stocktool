#include "scorecontroller.h"
#include <QTimer>
#include <QFile>
#include <QDir>
#include <QDesktopServices>
#include <QUrl>
#include "../Utility/ImPath.h"
#include "settingmanager.h"
#include "../stock/datamanager.h"

#define PREFIX_SPACE_COUNT  12
#define MIDDLE_SPACE_COUNT  4

DataScorer::DataScorer(QObject *parent)
    : QObject{parent}
{

}

void DataScorer::run()
{
    int lastMonth = -1;
    bool useLunarTime = m_scoreDataType == STOCK_DATA_YEAR || m_scoreDataType == STOCK_DATA_MONTH;
    for (QDate date=m_beginDate; date < m_endDate; date = date.addDays(1))
    {
        if (useLunarTime)
        {
            if (date.month() == lastMonth)
            {
                // 年和月的数据，按月去算就可以
                continue;
            }
        }
        else
        {
            // 周末不算
            int dayOfWeek = date.dayOfWeek();
            if (dayOfWeek == 6 || dayOfWeek == 7)
            {
                continue;
            }

            // 节假日也不算
            bool inHoliday = false;
            qint64 currentTime = date.startOfDay().toSecsSinceEpoch();
            for (const auto& holiday : SettingManager::getInstance()->m_holidays)
            {
                if (currentTime >= holiday.m_begin && currentTime <= holiday.m_end)
                {
                    inHoliday = true;
                    break;
                }
            }
            if (inHoliday)
            {
                continue;
            }
        }

        score(date);
        lastMonth = date.month();

        emit oneDayFinish();
    }
    emit runFinish(this);
}

void DataScorer::score(QDate date)
{
    QVector<StockData> stockDatas = getStockDatasByDate(date);
    static QVector<QString> words;
    if (words.empty())
    {
        words.append(QString::fromWCharArray(L"禄"));
        words.append(QString::fromWCharArray(L"权"));
        words.append(QString::fromWCharArray(L"科"));
        words.append(QString::fromWCharArray(L"忌"));
        words.append(QString::fromWCharArray(L"存"));
        words.append(QString::fromWCharArray(L"羊"));
    }

    for (auto& stockData : stockDatas)
    {
        ScoreResult scoreResult;
        scoreResult.m_stockName = stockData.m_stockName;
        scoreResult.m_beginTime = stockData.m_beginTime;

        bool oneGongs[2] = {true, false};
        for (int i=0; i<2; i++)
        {
            for (int j=0; j<words.size(); j++)
            {
                FilterCondition filterCondition;
                if (oneGongs[i])
                {
                    filterCondition.m_oneInclude = words[j];
                }
                else
                {
                    filterCondition.m_twoInclude = words[j];
                }
                bool ok = StockDataUtil::checkIfStockDataOk(stockData, filterCondition, m_matchAll);
                scoreResult.m_hasWords[i*words.size()+j] = ok;
            }
        }

        if (!scoreResult.isZero())
        {
            m_scoreResults.append(scoreResult);
        }
    }
}

QVector<StockData> DataScorer::getStockDatasByDate(const QDate& date)
{
    if (m_scoreDataType == STOCK_DATA_YEAR)
    {
        return getYearStockDatasByDate(date);
    }
    else if (m_scoreDataType == STOCK_DATA_MONTH)
    {
        return getMonthStockDatasByDate(date);
    }
    else
    {
        return getDayStockDatasByDate(date);
    }
}

QVector<StockData> DataScorer::getYearStockDatasByDate(const QDate& date)
{
    // 从前1年开始(考虑到农历公历时间差)，找到开始筛选的数据点
    QDateTime beginSearchDateTime;
    beginSearchDateTime.setDate(QDate(date.year()-1, 1, 1));
    qint64 beginSearchTime = beginSearchDateTime.toSecsSinceEpoch();

    const QVector<StockData>& stockDatas = DataManager::getInstance()->m_stockDatas[STOCK_DATA_YEAR];
    int left = findIndex(stockDatas, beginSearchTime);

    QString year = date.toString("yyyy");
    QVector<StockData> stockDataResult;
    while (left < stockDatas.length())
    {
        const StockData& currentStockData = stockDatas[left];
        if (currentStockData.m_lunarTime == year)
        {
            stockDataResult.append(currentStockData);
        }

        if (currentStockData.m_lunarTime.toInt() > date.year())
        {
            break;
        }

        left++;
    }

    return stockDataResult;
}

QVector<StockData> DataScorer::getMonthStockDatasByDate(const QDate& date)
{
    // 从前3个月开始(考虑到农历公历时间差)，找到开始筛选的数据点
    QDateTime beginSearchDateTime;
    QDate tempDate = date.addMonths(-3);
    beginSearchDateTime.setDate(QDate(tempDate.year(), tempDate.month(), 1));
    qint64 beginSearchTime = beginSearchDateTime.toSecsSinceEpoch();

    const QVector<StockData>& stockDatas = DataManager::getInstance()->m_stockDatas[STOCK_DATA_MONTH];
    int left = findIndex(stockDatas, beginSearchTime);

    // 开始筛选
    QString month = date.toString(QString::fromWCharArray(L"yyyy年M月"));
    QVector<StockData> stockDataResult;
    while (left < stockDatas.length())
    {
        const StockData& currentStockData = stockDatas[left];
        if (currentStockData.m_lunarTime == month)
        {
            stockDataResult.append(currentStockData);
        }

        QDateTime lunarDateTime = QDateTime::fromString(currentStockData.m_lunarTime, "yyyy年M月");
        if (lunarDateTime.date() > date)
        {
            break;
        }

        left++;
    }

    return stockDataResult;
}

QVector<StockData> DataScorer::getDayStockDatasByDate(const QDate& date)
{
    QDateTime beginSearchDateTime;
    beginSearchDateTime.setDate(date);
    qint64 beginSearchTime = beginSearchDateTime.toSecsSinceEpoch();

    const QVector<StockData>& stockDatas = DataManager::getInstance()->m_stockDatas[STOCK_DATA_DAY];
    int left = findIndex(stockDatas, beginSearchTime);

    // 开始筛选
    qint64 filterTime = beginSearchTime;
    QVector<StockData> stockDataResult;
    while (left < stockDatas.length())
    {
        const StockData& currentStockData = stockDatas[left];
        if (currentStockData.m_beginTime == filterTime)
        {
            stockDataResult.append(currentStockData);
        }

        if (currentStockData.m_beginTime > filterTime)
        {
            break;
        }

        left++;
    }

    return stockDataResult;
}

int DataScorer::findIndex(const QVector<StockData>& stockDatas, qint64 beginSearchTime)
{
    // 二分法查找
    int left = 0;
    int right = stockDatas.length();
    while (left <= right)
    {
        if (right - left < 100)
        {
            break;
        }

        int mid = (left + right) / 2;
        if (stockDatas[mid].m_beginTime < beginSearchTime)
        {
            left = mid;
        }
        else
        {
            right = mid;
        }
    }

    return left;
}


ScoreDataController::ScoreDataController(QObject *parent)
    : QObject{parent}
{

}

void ScoreDataController::run(const ScoreParam& scorerParam)
{
    if (scorerParam.m_name.isEmpty())
    {
        emit printLog(QString::fromWCharArray(L"得分器名字为空"));
        emit runFinish();
        return;
    }

    emit printLog(QString::fromWCharArray(L"开始筛选数据"));

    m_scoreParam = scorerParam;
    m_totalDays = scorerParam.m_beginDate.daysTo(scorerParam.m_endDate);
    if (m_totalDays <= 0)
    {
        qCritical("total days is zero");
        return;
    }

    // 开启多线程筛选数据
    int threadCount = QThread::idealThreadCount();
    threadCount = qMin(threadCount, m_totalDays);
    int dayPerThread = m_totalDays / threadCount;
    for (int i=0; i<threadCount; i++)
    {
        QDate begin = scorerParam.m_beginDate.addDays(i*dayPerThread);
        QDate end = scorerParam.m_beginDate.addDays((i+1)*dayPerThread);
        if (i == threadCount-1)
        {
            end = scorerParam.m_endDate;
        }

        DataScorer* dataScorer = new DataScorer();
        dataScorer->m_scoreDataType = scorerParam.m_dataType;
        dataScorer->m_matchAll = scorerParam.m_matchAll;
        dataScorer->m_beginDate = begin;
        dataScorer->m_endDate = end;

        QThread* thread = new QThread();
        dataScorer->moveToThread(thread);
        connect(thread, &QThread::started, dataScorer, &DataScorer::run);
        connect(thread, &QThread::finished, thread, &QThread::deleteLater);
        connect(dataScorer, &DataScorer::oneDayFinish, this, &ScoreDataController::onScoreOneDayFinish);
        connect(dataScorer, &DataScorer::runFinish, this, &ScoreDataController::onScoreRunFinish);
        m_dataScorers.append(dataScorer);
        m_dataScorerThreads.append(thread);
    }

    emit printLog(QString::fromWCharArray(L"开始筛选数据，使用线程数%1个").arg(threadCount));
    for (int i=0; i<m_dataScorerThreads.size(); i++)
    {
        m_dataScorerThreads[i]->start();
    }

    QTimer* timer = new QTimer(this);
    connect(timer, &QTimer::timeout, [this]() {
        emit printLog(QString::fromWCharArray(L"筛选进度：%1/%2").arg(QString::number(m_finishDays), QString::number(m_totalDays)));
    });
    timer->start(5000);
}

void ScoreDataController::onScoreOneDayFinish()
{
    m_finishDays++;
}

void ScoreDataController::onScoreRunFinish(DataScorer* dataScorer)
{
    m_scoreResults.append(dataScorer->m_scoreResults);
    for (int i=0; i<m_dataScorers.size(); i++)
    {
        if (m_dataScorers[i] == dataScorer)
        {
            dataScorer->deleteLater();
            m_dataScorers.remove(i);
            m_dataScorerThreads[i]->quit();
            m_dataScorerThreads.remove(i);
            break;
        }
    }

    if (m_dataScorers.empty())
    {
        if (m_scoreResults.isEmpty())
        {
            emit printLog(QString::fromWCharArray(L"筛选数据完成，没有数据。"));
        }
        else
        {
            emit printLog(QString::fromWCharArray(L"筛选数据完成"));
            emit printLog(QString::fromWCharArray(L"开始统计分数"));
            summaryScore();
        }

        emit runFinish();
    }
}

void ScoreDataController::summaryScore()
{
    // 按股票名称、时间排序
    std::sort(m_scoreResults.begin(), m_scoreResults.end(), [](const ScoreResult& a, const ScoreResult& b) {
        return (a.m_stockName < b.m_stockName
                || (a.m_stockName == b.m_stockName && a.m_beginTime < b.m_beginTime));
    });

    // 统计股票的分数
    QString stockName;
    qint64 beginTime = 0;
    QVector<StockScore> stockScores;
    for (const auto& scoreResult : m_scoreResults)
    {
        if (scoreResult.m_stockName != stockName)
        {
            // 一只新股票
            StockScore stockScore;
            stockScore.m_stockName = scoreResult.m_stockName;
            stockScores.append(stockScore);

            stockName = scoreResult.m_stockName;
            beginTime = 0;
        }

        if (scoreResult.m_beginTime == beginTime)
        {
            // 可能有重复
            continue;
        }

        int score = getStockScore(scoreResult);
        stockScores[stockScores.length()-1].m_score += score;
    }

    // 获取时间范围和总天数
    QString dateRangeString = getDateRangeString();

    // 按分数排序
    std::sort(stockScores.begin(), stockScores.end(), [this](const StockScore& a, const StockScore& b) {
        if (m_scoreParam.m_orderUp && a.m_score < b.m_score)
        {
            return true;
        }

        if (!m_scoreParam.m_orderUp && a.m_score > b.m_score)
        {
            return true;
        }

        return false;
    });

    // 构造输出内容
    m_resultString = "";
    for (const auto& stockScore : stockScores)
    {
        m_resultString += stockScore.m_stockName + " " + dateRangeString + " " + QString::number(stockScore.m_score) + "\r\n\r\n";
    }
}

int ScoreDataController::getStockScore(const ScoreResult& stockResult)
{
    int scores[2] = {0};
    for (int i=0; i<2; i++)
    {
        int gongCount = SCORE_COUNT/2;
        for (int j=0; j<gongCount; j++)
        {
            int pos = i*gongCount+j;
            if (stockResult.m_hasWords[pos])
            {
                if (SettingManager::getInstance()->m_scores[pos] > scores[i])
                {
                    scores[i] = SettingManager::getInstance()->m_scores[pos];
                }
            }
        }
    }

    if (scores[0] > 0 && scores[1] > 0)
    {
        if (m_scoreParam.m_isGetMax)
        {
            return qMax(scores[0], scores[1]);
        }
        else
        {
            int score = (int)((scores[0]+scores[1])*m_scoreParam.m_sumFactor*1.0f/100);
            return score;
        }
    }
    else
    {
        return scores[0]+scores[1];
    }
}

QString ScoreDataController::getDateRangeString()
{
    QString dateRange;
    QString totalDates;
    QDate beginDate = m_scoreParam.m_beginDate;
    QDate endDate = m_scoreParam.m_endDate.addDays(-1);  // 最后一天不含，减少1
    if (m_scoreParam.m_dataType == STOCK_DATA_YEAR)
    {
        dateRange = QString::fromWCharArray(L"%1年--%2年").arg(QString::number(beginDate.year()), QString::number(endDate.year()));
        int totalYears = endDate.year() - beginDate.year() + 1;
        totalDates = QString::fromWCharArray(L"共%1年").arg(totalYears);
    }
    else if (m_scoreParam.m_dataType == STOCK_DATA_MONTH)
    {
        dateRange = QString::fromWCharArray(L"%1年%2月--%3年%4月").arg(
                    QString::number(beginDate.year()), QString::number(beginDate.month()),
                    QString::number(endDate.year()), QString::number(endDate.month()));
        int totalMonths = endDate.year()*12+endDate.month() - beginDate.year()*12 - beginDate.month() + 1;
        totalDates = QString::fromWCharArray(L"共%1月").arg(totalMonths);
    }
    else
    {
        QString beginString = beginDate.toString(QString::fromWCharArray(L"yyyy年M月d日"));
        QString endString = endDate.toString(QString::fromWCharArray(L"yyyy年M月d日"));
        dateRange = beginString + "--" + endString;

        int totalDays = 0;
        for (QDate date=beginDate; date<=endDate; date=date.addDays(1))
        {
            // 周末不算
            int dayOfWeek = date.dayOfWeek();
            if (dayOfWeek == 6 || dayOfWeek == 7)
            {
                continue;
            }

            // 节假日也不算
            bool inHoliday = false;
            qint64 currentTime = date.startOfDay().toSecsSinceEpoch();
            for (const auto& holiday : SettingManager::getInstance()->m_holidays)
            {
                if (currentTime >= holiday.m_begin && currentTime <= holiday.m_end)
                {
                    inHoliday = true;
                    break;
                }
            }
            if (inHoliday)
            {
                continue;
            }

            totalDays++;
        }
        totalDates = QString::fromWCharArray(L"共%1日").arg(totalDays);
    }

    return dateRange + " " + totalDates;
}
