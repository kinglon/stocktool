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
                    && hasDayLineData(currentStockData, stockType)
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

QVector<StockData> DataFilterBase::getStockDataBySpecificDate()
{
    QVector<StockData> result;
    QVector<StockData>* stockDatas = nullptr;
    qint64 compareDate = SettingManager::getInstance()->m_compareDate;
    for (int i=STOCK_TYPE_GEGU; i<=STOCK_TYPE_ZHISHU; i++)
    {
        if (!SettingManager::getInstance()->m_stockSetting[i].m_enable)
        {
            continue;
        }

        if (SettingManager::getInstance()->m_compareContentFrom == COMPARE_CONTENT_ZHIDING_YEAR)
        {
            stockDatas = &StockDataManager::getInstance()->m_dataManagers[i]->m_stockDatas[STOCK_DATA_YEAR];
        }
        else if (SettingManager::getInstance()->m_compareContentFrom == COMPARE_CONTENT_ZHIDING_MONTH)
        {
            stockDatas = &StockDataManager::getInstance()->m_dataManagers[i]->m_stockDatas[STOCK_DATA_MONTH];
        }
        else if (SettingManager::getInstance()->m_compareContentFrom == COMPARE_CONTENT_ZHIDING_DAY)
        {
            stockDatas = &StockDataManager::getInstance()->m_dataManagers[i]->m_stockDatas[STOCK_DATA_DAY];
        }

        break;
    }

    if (stockDatas == nullptr)
    {
        return result;
    }

    qint64 searchTime = compareDate;
    int left = findIndex(*stockDatas, searchTime);
    while (left < stockDatas->length())
    {
        const StockData& currentStockData = (*stockDatas)[left];
        if (searchTime >= currentStockData.m_beginTime && searchTime <= currentStockData.m_endTime)
        {
            bool found = false;
            for (const auto& item : result)
            {
                if (item.m_industryName == currentStockData.m_industryName
                        && item.m_stockName == currentStockData.m_stockName)
                {
                    found = true;
                    break;
                }
            }

            if (!found)
            {
                result.append(currentStockData);
            }
        }

        if (currentStockData.m_beginTime > searchTime)
        {
            break;
        }

        left++;
    }

    return result;
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

bool DataFilterBase::hasDayLineData(const StockData& stockData, int stockType)
{
    const QVector<DayLineData>& stockDayLineData = StockDataManager::getInstance()->m_dataManagers[stockType]->m_dayLineDatas[STOCK_DATA_DAY];
    qint64 searchTime = stockData.m_beginTime;
    int left = findIndex(stockDayLineData, searchTime);
    while (left < stockDayLineData.length())
    {
        const DayLineData& currentDayLineData = stockDayLineData[left];
        if (currentDayLineData.m_beginTime == searchTime
                && currentDayLineData.m_industryName == stockData.m_industryName
                && currentDayLineData.m_stockName == stockData.m_stockName)
        {
            return true;
        }

        if (currentDayLineData.m_beginTime > searchTime)
        {
            break;
        }

        left++;
    }

    return false;
}

DataFilterKeyWord::DataFilterKeyWord(QObject *parent)
    : DataFilterBase{parent}
{
    int compareDataFrom = SettingManager::getInstance()->m_compareContentFrom;
    if (compareDataFrom == COMPARE_CONTENT_ZHIDING_YEAR
                 || compareDataFrom == COMPARE_CONTENT_ZHIDING_MONTH
                 || compareDataFrom == COMPARE_CONTENT_ZHIDING_DAY)
    {
        m_compareStockDatas = getStockDataBySpecificDate();
    }
}

bool DataFilterKeyWord::canMatch(const StockData& stockData)
{
    QString oneGongCompareContent;
    QString twoGongCompareContent;
    int compareDataFrom = SettingManager::getInstance()->m_compareContentFrom;
    if (compareDataFrom == COMPARE_CONTENT_ZHIDING_WORD)
    {
        oneGongCompareContent = SettingManager::getInstance()->m_zhiDingWordFilterCondition.m_oneInclude;
        twoGongCompareContent = SettingManager::getInstance()->m_zhiDingWordFilterCondition.m_twoInclude;
    }
    else if (compareDataFrom == COMPARE_CONTENT_ZHIDING_YEAR
             || compareDataFrom == COMPARE_CONTENT_ZHIDING_MONTH
             || compareDataFrom == COMPARE_CONTENT_ZHIDING_DAY)
    {
        for (const StockData& item : m_compareStockDatas)
        {
            if (item.m_industryName == stockData.m_industryName
                    && item.m_stockName == stockData.m_stockName)
            {
                oneGongCompareContent = item.m_data[0];
                twoGongCompareContent = item.m_data[1];
                break;
            }
        }
    }

    if (oneGongCompareContent.isEmpty() && twoGongCompareContent.isEmpty())
    {
        return false;
    }

    if (m_equalFind)
    {
        if (stockData.m_data[0] == oneGongCompareContent
            && stockData.m_data[1] == twoGongCompareContent)
        {
            return true;
        }
    }
    else
    {
        if (stockData.m_data[0].indexOf(oneGongCompareContent) >= 0
            && stockData.m_data[1].indexOf(twoGongCompareContent) >= 0)
        {
            return true;
        }
    }

    return false;
}

DataFilterAlgorithm::DataFilterAlgorithm(QObject *parent)
    : DataFilterBase{parent}
{
    if (SettingManager::getInstance()->enableDebugLog())
    {
        m_stockDataUtil.enableDebugLog();
    }

    int compareDataFrom = SettingManager::getInstance()->m_compareContentFrom;
    if (compareDataFrom == COMPARE_CONTENT_ZHIDING_YEAR
                 || compareDataFrom == COMPARE_CONTENT_ZHIDING_MONTH
                 || compareDataFrom == COMPARE_CONTENT_ZHIDING_DAY)
    {
        m_compareStockDatas = getStockDataBySpecificDate();
    }

    m_allStatus.append(QString::fromWCharArray(L"阴禄"));
    m_allStatus.append(QString::fromWCharArray(L"阴权"));
    m_allStatus.append(QString::fromWCharArray(L"阴科"));
    m_allStatus.append(QString::fromWCharArray(L"阴忌"));
    m_allStatus.append(QString::fromWCharArray(L"阴"));

    m_allStatus.append(QString::fromWCharArray(L"阳禄"));
    m_allStatus.append(QString::fromWCharArray(L"阳权"));
    m_allStatus.append(QString::fromWCharArray(L"阳科"));
    m_allStatus.append(QString::fromWCharArray(L"阳忌"));
    m_allStatus.append(QString::fromWCharArray(L"阳"));

    m_allStatus.append(QString::fromWCharArray(L"武禄"));
    m_allStatus.append(QString::fromWCharArray(L"武权"));
    m_allStatus.append(QString::fromWCharArray(L"武科"));
    m_allStatus.append(QString::fromWCharArray(L"武忌"));
    m_allStatus.append(QString::fromWCharArray(L"武"));

    m_allStatus.append(QString::fromWCharArray(L"破禄"));
    m_allStatus.append(QString::fromWCharArray(L"破权"));
    m_allStatus.append(QString::fromWCharArray(L"破"));

    m_allStatus.append(QString::fromWCharArray(L"机禄"));
    m_allStatus.append(QString::fromWCharArray(L"机权"));
    m_allStatus.append(QString::fromWCharArray(L"机科"));
    m_allStatus.append(QString::fromWCharArray(L"机忌"));
    m_allStatus.append(QString::fromWCharArray(L"机"));

    m_allStatus.append(QString::fromWCharArray(L"紫科"));
    m_allStatus.append(QString::fromWCharArray(L"紫权"));
    m_allStatus.append(QString::fromWCharArray(L"紫"));

    m_allStatus.append(QString::fromWCharArray(L"府科"));
    m_allStatus.append(QString::fromWCharArray(L"府"));

    m_allStatus.append(QString::fromWCharArray(L"狼禄"));
    m_allStatus.append(QString::fromWCharArray(L"狼权"));
    m_allStatus.append(QString::fromWCharArray(L"狼忌"));
    m_allStatus.append(QString::fromWCharArray(L"狼"));

    m_allStatus.append(QString::fromWCharArray(L"巨禄"));
    m_allStatus.append(QString::fromWCharArray(L"巨权"));
    m_allStatus.append(QString::fromWCharArray(L"巨忌"));
    m_allStatus.append(QString::fromWCharArray(L"巨"));

    m_allStatus.append(QString::fromWCharArray(L"梁禄"));
    m_allStatus.append(QString::fromWCharArray(L"梁权"));
    m_allStatus.append(QString::fromWCharArray(L"梁科"));
    m_allStatus.append(QString::fromWCharArray(L"梁"));

    m_allStatus.append(QString::fromWCharArray(L"同禄"));
    m_allStatus.append(QString::fromWCharArray(L"同权"));
    m_allStatus.append(QString::fromWCharArray(L"同忌"));
    m_allStatus.append(QString::fromWCharArray(L"同"));

    m_allStatus.append(QString::fromWCharArray(L"廉禄"));
    m_allStatus.append(QString::fromWCharArray(L"廉忌"));
    m_allStatus.append(QString::fromWCharArray(L"廉"));

    m_allStatus.append(QString::fromWCharArray(L"相"));

    m_allStatus.append(QString::fromWCharArray(L"杀"));

    m_allStatus.append(QString::fromWCharArray(L"昌科"));
    m_allStatus.append(QString::fromWCharArray(L"昌忌"));
    m_allStatus.append(QString::fromWCharArray(L"昌"));

    m_allStatus.append(QString::fromWCharArray(L"曲科"));
    m_allStatus.append(QString::fromWCharArray(L"曲忌"));
    m_allStatus.append(QString::fromWCharArray(L"曲"));

    m_allStatus.append(QString::fromWCharArray(L"羊"));

    m_allStatus.append(QString::fromWCharArray(L"存"));
}

bool DataFilterAlgorithm::canMatch(const StockData& stockData)
{
    bool matchAll = SettingManager::getInstance()->m_allMatch;

    // 获取一二宫含与不含的内容
    QString oneInclude;
    QString oneExclude;
    QString twoInclude;
    QString twoExclude;
    int compareDataFrom = SettingManager::getInstance()->m_compareContentFrom;
    if (compareDataFrom == COMPARE_CONTENT_ZHIDING_WORD)
    {
        oneInclude = SettingManager::getInstance()->m_zhiDingWordFilterCondition.m_oneInclude;
        twoInclude = SettingManager::getInstance()->m_zhiDingWordFilterCondition.m_twoInclude;
    }
    else if (compareDataFrom == COMPARE_CONTENT_ZHIDING_YEAR
             || compareDataFrom == COMPARE_CONTENT_ZHIDING_MONTH
             || compareDataFrom == COMPARE_CONTENT_ZHIDING_DAY)
    {
        for (const StockData& item : m_compareStockDatas)
        {
            if (item.m_industryName == stockData.m_industryName
                    && item.m_stockName == stockData.m_stockName)
            {
                m_stockDataUtil.transformStockData(item, oneInclude, twoInclude, matchAll);
                break;
            }
        }
    }
    else if (compareDataFrom == COMPARE_CONTENT_ALL_WORD)
    {
        oneInclude = SettingManager::getInstance()->m_allWordFilterCondition.m_oneInclude;
        twoInclude = SettingManager::getInstance()->m_allWordFilterCondition.m_twoInclude;
        oneExclude = SettingManager::getInstance()->m_allWordFilterCondition.m_oneExclude;
        twoExclude = SettingManager::getInstance()->m_allWordFilterCondition.m_twoExclude;
    }

    FilterConditionV3 filterCondition;
    filterCondition.m_oneIncludes = extractStatus(oneInclude);
    filterCondition.m_twoIncludes = extractStatus(twoInclude);
    filterCondition.m_oneExcludes = extractStatus(oneExclude);
    filterCondition.m_twoExcludes = extractStatus(twoExclude);
    if (!filterCondition.isEnable())
    {
        return false;
    }

    // 开始检查
    return m_stockDataUtil.checkIfStockDataOk(stockData, filterCondition, matchAll);
}

QVector<QString> DataFilterAlgorithm::extractStatus(const QString& content2)
{
    QVector<QString> result;
    if (content2.isEmpty())
    {
        return result;
    }

    QString content = content2;
    for (const auto& status : m_allStatus)
    {
        if (content.indexOf(status) >= 0)
        {
            result.append(status);
            content.replace(status, "");
        }
    }

    return result;
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
            dataFilter = new DataFilterKeyWord();
            ((DataFilterKeyWord*)dataFilter)->m_equalFind = true;
        }
        else if (searchMethod == SEARCH_TYPE_KEY_WORD)
        {
            dataFilter = new DataFilterKeyWord();
            ((DataFilterKeyWord*)dataFilter)->m_equalFind = false;
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
