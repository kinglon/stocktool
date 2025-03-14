#include "filterdatacontroller.h"
#include <QThread>
#include <QTimer>

DataFilterBase::DataFilterBase(QObject *parent)
    : QObject{parent}
{

}

void DataFilterBase::run()
{
    QVector<StockData>* stockDatas = nullptr;
    int stockType = STOCK_TYPE_GEGU;
    for (int i=STOCK_TYPE_GEGU; i<=STOCK_TYPE_ZHISHU; i++)
    {
        if (SettingManager::getInstance()->m_stockSetting[i].m_enable)
        {
            DataManager* dataManager = StockDataManager::getInstance()->m_dataManagers[i];
            stockDatas = &dataManager->m_stockDatas[STOCK_DATA_DAY];
            stockType = i;
            break;
        }
    }

    if (stockDatas == nullptr || stockDatas->empty())
    {
        emit runFinish(this);
        return;
    }

    for (QDate date=m_beginDate; date < m_endDate; date = date.addDays(1))
    {
        QDateTime searchDate;
        searchDate.setDate(date);
        qint64 searchTime = searchDate.toSecsSinceEpoch();
        int left = findIndex(*stockDatas, searchTime);
        while (left < stockDatas->length())
        {
            const StockData& currentStockData = (*stockDatas)[left];
            if (currentStockData.m_beginTime == searchTime
                    && canMatch(currentStockData)
                    && isZhangFuOk(currentStockData, stockType))
            {
                m_stockDatas.append(currentStockData);
            }

            if (currentStockData.m_beginTime > searchTime)
            {
                break;
            }

            left++;
        }

        emit oneDayFinish();
    }
    emit runFinish(this);
}

int DataFilterBase::findIndex(const QVector<StockData>& stockDatas, qint64 beginSearchTime)
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

int DataFilterBase::findIndex(const QVector<DayLineData>& dayLineDatas, qint64 beginSearchTime)
{
    // 二分法查找
    int left = 0;
    int right = dayLineDatas.length();
    while (left <= right)
    {
        if (right - left < 100)
        {
            break;
        }

        int mid = (left + right) / 2;
        if (dayLineDatas[mid].m_beginTime < beginSearchTime)
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


bool DataFilterBase::isZhangFuOk(const StockData& stockData, int stockType)
{
    // 获取股票的涨幅
    const StockSetting& stockSetting = SettingManager::getInstance()->m_stockSetting[stockType];
    if (stockSetting.m_enableZhangDie)
    {
        const QVector<DayLineData>& stockDayLineData = StockDataManager::getInstance()->m_dataManagers[stockType]->m_dayLineDatas[STOCK_DATA_DAY];
        qint64 searchTime = stockData.m_beginTime;
        int left = findIndex(stockDayLineData, searchTime);
        while (left < stockDayLineData.length())
        {
            const DayLineData& currentDayLineData = stockDayLineData[left];
            if (currentDayLineData.m_beginTime == searchTime
                    && currentDayLineData.m_industryName == stockData.m_industryName
                    && currentDayLineData.m_stockName == stockData.m_stockName
                    && (currentDayLineData.m_zhangFu < stockSetting.m_zhangDieStart || currentDayLineData.m_zhangFu > stockSetting.m_zhangDieEnd))
            {
                return false;
            }

            if (currentDayLineData.m_beginTime > searchTime)
            {
                break;
            }

            left++;
        }
    }

    for (int i=stockType+1; i<=STOCK_TYPE_ZHISHU; i++)
    {
        const StockSetting& stockSetting = SettingManager::getInstance()->m_stockSetting[i];
        if (stockSetting.m_enableZhangDie)
        {
            const QVector<DayLineData>& stockDayLineData = StockDataManager::getInstance()->m_dataManagers[i]->m_dayLineDatas[STOCK_DATA_DAY];
            qint64 searchTime = stockData.m_beginTime;
            int left = findIndex(stockDayLineData, searchTime);
            while (left < stockDayLineData.length())
            {
                const DayLineData& currentDayLineData = stockDayLineData[left];
                if (currentDayLineData.m_beginTime == searchTime
                        && (currentDayLineData.m_zhangFu < stockSetting.m_zhangDieStart || currentDayLineData.m_zhangFu > stockSetting.m_zhangDieEnd))
                {
                    return false;
                }

                if (currentDayLineData.m_beginTime > searchTime)
                {
                    break;
                }

                left++;
            }
        }
    }

    return true;
}

DataFilterJingQue::DataFilterJingQue(QObject *parent)
    : DataFilterBase{parent}
{

}

bool DataFilterJingQue::canMatch(const StockData& stockData)
{
    // todo by yejinlong
    return false;
}

DataFilterKeyWord::DataFilterKeyWord(QObject *parent)
    : DataFilterBase{parent}
{

}

bool DataFilterKeyWord::canMatch(const StockData& stockData)
{
    // todo by yejinlong
    return false;
}

DataFilterAlgorithm::DataFilterAlgorithm(QObject *parent)
    : DataFilterBase{parent}
{

}

bool DataFilterAlgorithm::canMatch(const StockData& stockData)
{
    // todo by yejinlong
    return false;
}


FilterDataController::FilterDataController(QObject *parent)
    : QObject{parent}
{

}

void FilterDataController::run()
{
    QDate beginDate = QDateTime::fromSecsSinceEpoch(SettingManager::getInstance()->m_recgDateStart).date();
    QDate endDate = QDateTime::fromSecsSinceEpoch(SettingManager::getInstance()->m_recgDateEnd).date();
    m_totalDays = beginDate.daysTo(endDate);
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
        QDate begin = beginDate.addDays(i*dayPerThread);
        QDate end = beginDate.addDays((i+1)*dayPerThread);
        if (i == threadCount-1)
        {
            end = endDate;
        }

        DataFilterBase* dataFilter = nullptr;
        int searchMethod = SettingManager::getInstance()->m_searchMethod;
        if (searchMethod == SEARCH_TYPE_JINGQUE)
        {
            dataFilter = new DataFilterJingQue();
        }
        else if (searchMethod == SEARCH_TYPE_KEY_WORD)
        {
            dataFilter = new DataFilterKeyWord();
        }
        else if (searchMethod == SEARCH_TYPE_ALGORITHM)
        {
            dataFilter = new DataFilterAlgorithm();
        }

        if (dataFilter == nullptr)
        {
            qCritical("wrong search method");
            return;
        }

        dataFilter->m_beginDate = begin;
        dataFilter->m_endDate = end;

        QThread* thread = new QThread();
        dataFilter->moveToThread(thread);
        connect(thread, &QThread::started, dataFilter, &DataFilterBase::run);
        connect(thread, &QThread::finished, thread, &QThread::deleteLater);
        connect(dataFilter, &DataFilterBase::oneDayFinish, this, &FilterDataController::onFilterOneDayFinish);
        connect(dataFilter, &DataFilterBase::runFinish, this, &FilterDataController::onFilterRunFinish);
        m_dataFilters.append(dataFilter);
        m_dataFilterThreads.append(thread);
    }

    for (int i=0; i<m_dataFilterThreads.size(); i++)
    {
        m_dataFilterThreads[i]->start();
    }

    QTimer* timer = new QTimer(this);
    connect(timer, &QTimer::timeout, [this]() {
        emit printLog(QString::fromWCharArray(L"筛选进度：%1/%2").arg(QString::number(m_finishDays), QString::number(m_totalDays)));
    });
    timer->start(5000);
}

void FilterDataController::onFilterOneDayFinish()
{
    m_finishDays++;
}

void FilterDataController::onFilterRunFinish(DataFilterBase* dataFilter)
{
    m_stockDatas.append(dataFilter->m_stockDatas);
    for (int i=0; i<m_dataFilters.size(); i++)
    {
        if (m_dataFilters[i] == dataFilter)
        {
            dataFilter->deleteLater();
            m_dataFilters.remove(i);
            m_dataFilterThreads[i]->quit();
            m_dataFilterThreads.remove(i);
            break;
        }
    }

    if (m_dataFilters.empty())
    {
        emit runFinish();
    }
}
