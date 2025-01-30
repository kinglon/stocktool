#include "filterdatacontroller.h"
#include <QTimer>
#include <QFile>
#include <QDir>
#include <QDesktopServices>
#include <QUrl>
#include "Utility/ImPath.h"
#include "settingmanager.h"
#include "datamanager.h"
#include "mergedatacontroller.h"

#define PREFIX_SPACE_COUNT  12
#define MIDDLE_SPACE_COUNT  4

DataFilter::DataFilter(QObject *parent)
    : QObject{parent}
{

}

void DataFilter::run()
{
    int lastMonth = -1;
    for (QDate date=m_beginDate; date < m_endDate; date = date.addDays(1))
    {
        if (m_notLossFilter)
        {
            filterByNotLoss(date);
        }
        else
        {
            if (m_onlyFilterHour)
            {
                filterOnlyHourData(date);
            }
            else
            {
                if (m_onlyFilterToMonth)
                {
                    if (date.month() != lastMonth)
                    {
                        lastMonth = date.month();
                        filterOnlyToMonth(date);
                    }
                }
                else
                {
                    filterNotOnlyToMonth(date);
                }
            }
        }
        emit oneDayFinish();
    }
    emit runFinish(this);
}

void DataFilter::filterByNotLoss(QDate date)
{
    // 无损筛选：关键词筛选
    QVector<StockData> emptyStockDatas;
    QVector<StockData> stockDatas;
    if (m_notLossFilterDataType == STOCK_DATA_YEAR)
    {
        filterYearData(date, true, true, stockDatas);
    }
    else if (m_notLossFilterDataType == STOCK_DATA_MONTH)
    {
        filterMonthData(date, true, true, emptyStockDatas, stockDatas);
    }
    else if (m_notLossFilterDataType == STOCK_DATA_DAY)
    {
        filterDayData(date, true, emptyStockDatas, stockDatas);
    }
    else if (m_notLossFilterDataType == STOCK_DATA_HOUR)
    {
        QVector<StockData> yiStockDatas;
        filterHourData(date, true, STOCK_DATA_HOUR_YI, YI_HOUR_FILTER_CONDTION, yiStockDatas);

        QVector<StockData> wuStockDatas;
        filterHourData(date, true, STOCK_DATA_HOUR_WU, WU_HOUR_FILTER_CONDTION, wuStockDatas);

        QVector<StockData> weiStockDatas;
        filterHourData(date, true, STOCK_DATA_HOUR_WEI, WEI_HOUR_FILTER_CONDTION, weiStockDatas);

        // 求交集
        for (int i=0; i<yiStockDatas.size(); i++)
        {
            for (int j=0; j<wuStockDatas.size(); j++)
            {
                if (yiStockDatas[i].m_stockName == wuStockDatas[j].m_stockName)
                {
                    for (int k=0; k<weiStockDatas.size(); k++)
                    {
                        if (yiStockDatas[i].m_stockName == weiStockDatas[k].m_stockName)
                        {
                            stockDatas.append(yiStockDatas[i]);
                            break;
                        }
                    }
                    break;
                }
            }
        }
    }

    m_stockDatas.append(stockDatas);
}

void DataFilter::filterOnlyToMonth(QDate date)
{
    // 时间按农历去筛选
    // 筛选年数据
    QVector<StockData> yearStockDatas;
    if (!SettingManager::getInstance()->m_filterCondition[YEAR_FILTER_CONDTION].isEnable())
    {
        // 没设置筛选条件，取全部
        yearStockDatas = DataManager::getInstance()->m_stockDatas[STOCK_DATA_YEAR];
    }
    else
    {
        filterYearData(date, false, true, yearStockDatas);
    }

    if (yearStockDatas.empty())
    {
        return;
    }    

    // 筛选月数据
    QVector<StockData> monthStockDatas;
    filterMonthData(date, false, true, yearStockDatas, monthStockDatas);
    m_stockDatas.append(monthStockDatas);
}

void DataFilter::filterNotOnlyToMonth(QDate date)
{
    // 时间按公历去筛选
    // 筛选年数据
    QVector<StockData> yearStockDatas;
    if (!SettingManager::getInstance()->m_filterCondition[YEAR_FILTER_CONDTION].isEnable())
    {
        // 没设置筛选条件，取全部
        yearStockDatas = DataManager::getInstance()->m_stockDatas[STOCK_DATA_YEAR];
    }
    else
    {
        filterYearData(date, false, false, yearStockDatas);
    }

    if (yearStockDatas.empty())
    {
        return;
    }

    // 筛选月数据
    QVector<StockData> monthStockDatas;
    if (!SettingManager::getInstance()->m_filterCondition[MONTH_FILTER_CONDTION].isEnable())
    {
        monthStockDatas = yearStockDatas;
    }
    else
    {
        filterMonthData(date, false, false, yearStockDatas, monthStockDatas);
    }
    if (monthStockDatas.empty())
    {
        return;
    }
    yearStockDatas.clear();

    // 筛选日数据
    QVector<StockData> dayStockDatas;
    if (!SettingManager::getInstance()->m_filterCondition[DAY_FILTER_CONDTION].isEnable())
    {
        dayStockDatas = monthStockDatas;
    }
    else
    {
        filterDayData(date, false, monthStockDatas, dayStockDatas);
    }
    if (dayStockDatas.empty())
    {
        return;
    }
    monthStockDatas.clear();

    // 筛选第2日数据
    QVector<StockData> secondDayStockDatas;
    if (!SettingManager::getInstance()->m_filterCondition[SECOND_DAY_FILTER_CONDTION].isEnable())
    {
        secondDayStockDatas = dayStockDatas;
    }
    else
    {
        filterSecondDayData(date, dayStockDatas, secondDayStockDatas);
    }
    if (secondDayStockDatas.empty())
    {
        return;
    }
    dayStockDatas.clear();

    // 筛选已时数据
    QVector<StockData> yiHourStockDatas;
    if (!SettingManager::getInstance()->m_filterCondition[YI_HOUR_FILTER_CONDTION].isEnable())
    {
        yiHourStockDatas = secondDayStockDatas;
    }
    else
    {
        filterHourData(date, secondDayStockDatas, STOCK_DATA_HOUR_YI, YI_HOUR_FILTER_CONDTION, yiHourStockDatas);
    }
    if (yiHourStockDatas.empty())
    {
        return;
    }
    secondDayStockDatas.clear();

    // 筛选午时数据
    QVector<StockData> wuHourStockDatas;
    if (!SettingManager::getInstance()->m_filterCondition[WU_HOUR_FILTER_CONDTION].isEnable())
    {
        wuHourStockDatas = yiHourStockDatas;
    }
    else
    {
        filterHourData(date, yiHourStockDatas, STOCK_DATA_HOUR_WU, WU_HOUR_FILTER_CONDTION, wuHourStockDatas);
    }
    if (wuHourStockDatas.empty())
    {
        return;
    }
    yiHourStockDatas.clear();

    // 筛选未时数据
    QVector<StockData> weiHourStockDatas;
    filterHourData(date, wuHourStockDatas, STOCK_DATA_HOUR_WEI, WEI_HOUR_FILTER_CONDTION, weiHourStockDatas);
    m_stockDatas.append(weiHourStockDatas);
}

void DataFilter::filterOnlyHourData(QDate date)
{
    // 筛选已时数据
    QVector<StockData> yiHourStockDatas;
    filterHourData(date, false, STOCK_DATA_HOUR_YI, YI_HOUR_FILTER_CONDTION, yiHourStockDatas);
    m_stockDatas.append(yiHourStockDatas);

    // 筛选午时数据
    QVector<StockData> wuHourStockDatas;
    filterHourData(date, false, STOCK_DATA_HOUR_WU, WU_HOUR_FILTER_CONDTION, wuHourStockDatas);
    m_stockDatas.append(wuHourStockDatas);


    // 筛选未时数据
    QVector<StockData> weiHourStockDatas;
    filterHourData(date, false, STOCK_DATA_HOUR_WEI, WEI_HOUR_FILTER_CONDTION, weiHourStockDatas);
    m_stockDatas.append(weiHourStockDatas);
}

int DataFilter::findIndex(const QVector<StockData>& stockDatas, qint64 beginSearchTime)
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

void DataFilter::filterYearData(QDate date, bool notLossFilter, bool useLunarTime, QVector<StockData>& yearStockDatas)
{
    // 从前1年开始(考虑到农历公历时间差)，找到开始筛选的数据点
    QDateTime beginSearchDateTime;
    beginSearchDateTime.setDate(QDate(date.year()-1, 1, 1));
    qint64 beginSearchTime = beginSearchDateTime.toSecsSinceEpoch();

    const QVector<StockData>& stockDatas = DataManager::getInstance()->m_stockDatas[STOCK_DATA_YEAR];
    int left = findIndex(stockDatas, beginSearchTime);

    // 开始筛选
    const FilterCondition& yearFilterCondition = SettingManager::getInstance()->m_filterCondition[YEAR_FILTER_CONDTION];
    const FilterConditionV2& filterConditionV2 = SettingManager::getInstance()->m_filterConditionV2;
    QString year = date.toString("yyyy");
    while (left < stockDatas.length())
    {
        const StockData& currentStockData = stockDatas[left];
        if (!notLossFilter)
        {
            if (useLunarTime)
            {
                if (currentStockData.m_lunarTime == year)
                {
                    if (StockDataUtil::checkIfStockDataOk(currentStockData, yearFilterCondition, m_matchAll))
                    {
                        yearStockDatas.append(currentStockData);
                    }
                }

                if (currentStockData.m_lunarTime.toInt() > date.year())
                {
                    break;
                }
            }
            else
            {
                QDateTime beginDateTime = QDateTime::fromSecsSinceEpoch(currentStockData.m_beginTime);
                QString beginYear = beginDateTime.toString("yyyy");
                QString endYear = QDateTime::fromSecsSinceEpoch(currentStockData.m_endTime).toString("yyyy");
                if (beginYear == year || endYear == year)
                {
                    if (StockDataUtil::checkIfStockDataOk(currentStockData, yearFilterCondition, m_matchAll))
                    {
                        yearStockDatas.append(currentStockData);
                    }
                }

                if (beginDateTime.date().year() > date.year())
                {
                    break;
                }
            }
        }
        else
        {
            if (currentStockData.m_lunarTime == year)
            {
                if (StockDataUtil::checkIfStockDataOkV2(currentStockData, filterConditionV2))
                {
                    yearStockDatas.append(currentStockData);
                }
            }

            if (currentStockData.m_lunarTime.toInt() > date.year())
            {
                break;
            }
        }

        left++;
    }
}

void DataFilter::filterMonthData(QDate date, bool notLossFilter, bool useLunarTime, const QVector<StockData>& yearStockDatas, QVector<StockData>& monthStockDatas)
{
    // 从前3个月开始(考虑到农历公历时间差)，找到开始筛选的数据点
    QDateTime beginSearchDateTime;
    QDate tempDate = date.addMonths(-3);
    beginSearchDateTime.setDate(QDate(tempDate.year(), tempDate.month(), 1));
    qint64 beginSearchTime = beginSearchDateTime.toSecsSinceEpoch();

    const QVector<StockData>& stockDatas = DataManager::getInstance()->m_stockDatas[STOCK_DATA_MONTH];
    int left = findIndex(stockDatas, beginSearchTime);

    // 开始筛选
    const FilterCondition& monthFilterCondition = SettingManager::getInstance()->m_filterCondition[MONTH_FILTER_CONDTION];
    const FilterConditionV2& filterConditionV2 = SettingManager::getInstance()->m_filterConditionV2;
    QString month = date.toString(QString::fromWCharArray(L"yyyy年M月"));
    QDateTime filterDateTime;
    filterDateTime.setDate(QDate(date.year(), date.month(), 1));
    qint64 filterTime = filterDateTime.toSecsSinceEpoch();
    while (left < stockDatas.length())
    {
        const StockData& currentStockData = stockDatas[left];
        if (notLossFilter)
        {
            if (currentStockData.m_lunarTime == month)
            {
                if (StockDataUtil::checkIfStockDataOkV2(currentStockData, filterConditionV2))
                {
                    monthStockDatas.append(currentStockData);
                }
            }

            QDateTime lunarDateTime = QDateTime::fromString(currentStockData.m_lunarTime, "yyyy年M月");
            if (lunarDateTime.date() > date)
            {
                break;
            }
        }
        else
        {
            bool ok = false;
            if (useLunarTime)
            {
                if (currentStockData.m_lunarTime == month)
                {
                    if (StockDataUtil::checkIfStockDataOk(currentStockData, monthFilterCondition, m_matchAll))
                    {
                        ok = true;
                    }
                }

                QDateTime lunarDateTime = QDateTime::fromString(currentStockData.m_lunarTime, "yyyy年M月");
                if (lunarDateTime.date() > date)
                {
                    break;
                }
            }
            else
            {
                if (filterTime >= currentStockData.m_beginTime && filterTime <= currentStockData.m_endTime)
                {
                    if (StockDataUtil::checkIfStockDataOk(currentStockData, monthFilterCondition, m_matchAll))
                    {
                        ok = true;
                    }
                }

                if (currentStockData.m_beginTime > filterTime)
                {
                    break;
                }
            }

            if (ok)
            {
                for (const auto& stockData : yearStockDatas)
                {
                    if (currentStockData.m_stockName == stockData.m_stockName)
                    {
                        monthStockDatas.append(currentStockData);
                        break;
                    }
                }
            }
        }

        left++;
    }
}

void DataFilter::filterDayData(QDate date, bool notLossFilter, const QVector<StockData>& monthStockDatas, QVector<StockData>& dayStockDatas)
{
    QDateTime beginSearchDateTime;
    beginSearchDateTime.setDate(date);
    qint64 beginSearchTime = beginSearchDateTime.toSecsSinceEpoch();

    const QVector<StockData>& stockDatas = DataManager::getInstance()->m_stockDatas[STOCK_DATA_DAY];
    int left = findIndex(stockDatas, beginSearchTime);

    // 开始筛选
    const FilterCondition& dayFilterCondition = SettingManager::getInstance()->m_filterCondition[DAY_FILTER_CONDTION];
    const FilterConditionV2& filterConditionV2 = SettingManager::getInstance()->m_filterConditionV2;
    qint64 filterTime = beginSearchTime;
    while (left < stockDatas.length())
    {
        const StockData& currentStockData = stockDatas[left];        
        bool ok = false;
        if (currentStockData.m_beginTime == filterTime)
        {
            if (notLossFilter)
            {
                ok = StockDataUtil::checkIfStockDataOkV2(currentStockData, filterConditionV2);
            }
            else
            {
                ok = StockDataUtil::checkIfStockDataOk(currentStockData, dayFilterCondition, m_matchAll);
            }
        }

        if (currentStockData.m_beginTime > filterTime)
        {
            break;
        }

        if (ok)
        {
            if (notLossFilter)
            {
                dayStockDatas.append(currentStockData);
            }
            else
            {
                for (const auto& stockData : monthStockDatas)
                {
                    if (currentStockData.m_stockName == stockData.m_stockName)
                    {
                        dayStockDatas.append(currentStockData);
                        break;
                    }
                }
            }
        }

        left++;
    }
}

void DataFilter::filterSecondDayData(QDate date, const QVector<StockData>& dayStockDatas, QVector<StockData>& secondDayStockDatas)
{
    QDateTime beginSearchDateTime;
    beginSearchDateTime.setDate(date.addDays(1));
    qint64 beginSearchTime = beginSearchDateTime.toSecsSinceEpoch();

    const QVector<StockData>& stockDatas = DataManager::getInstance()->m_stockDatas[STOCK_DATA_DAY];
    int left = findIndex(stockDatas, beginSearchTime);

    // 开始筛选
    FilterCondition filterCondition = SettingManager::getInstance()->m_filterCondition[SECOND_DAY_FILTER_CONDTION];
    qint64 filterTime = beginSearchTime;
    QVector<StockData> okStockDatas;
    while (left < stockDatas.length())
    {
        const StockData& currentStockData = stockDatas[left];
        if (currentStockData.m_beginTime == filterTime)
        {
            if (StockDataUtil::checkIfStockDataOk(currentStockData, filterCondition, m_matchAll))
            {
                okStockDatas.append(currentStockData);
            }
        }

        if (currentStockData.m_beginTime > filterTime)
        {
            break;
        }

        left++;
    }

    for (const auto& dayStockData : dayStockDatas)
    {
        for (const auto& okStockData : okStockDatas)
        {
            if (dayStockData.m_stockName == okStockData.m_stockName)
            {
                secondDayStockDatas.append(dayStockData);
                break;
            }
        }
    }
}

void DataFilter::filterHourData(QDate date, const QVector<StockData>& matchStockDatas, int dataType, int filterType, QVector<StockData>& hourStockDatas)
{
    QVector<StockData> tempHourStockDatas;
    filterHourData(date, false, dataType, filterType, tempHourStockDatas);
    for (const auto& currentStockData : tempHourStockDatas)
    {
        for (const auto& stockData : matchStockDatas)
        {
            if (currentStockData.m_stockName == stockData.m_stockName)
            {
                hourStockDatas.append(currentStockData);
                break;
            }
        }
    }
}

void DataFilter::filterHourData(QDate date, bool notLossFilter, int dataType, int filterType, QVector<StockData>& hourStockDatas)
{
    QDateTime beginSearchDateTime;
    beginSearchDateTime.setDate(date);
    qint64 beginSearchTime = beginSearchDateTime.toSecsSinceEpoch();

    const QVector<StockData>& stockDatas = DataManager::getInstance()->m_stockDatas[dataType];
    int left = findIndex(stockDatas, beginSearchTime);

    // 开始筛选
    qint64 filterTime = beginSearchTime;
    while (left < stockDatas.length())
    {
        const StockData& currentStockData = stockDatas[left];
        bool ok = false;
        if (currentStockData.m_beginTime == filterTime)
        {
            if (notLossFilter)
            {
                ok = StockDataUtil::checkIfStockDataOkV2(currentStockData, SettingManager::getInstance()->m_filterConditionV2);
            }
            else
            {
                ok = StockDataUtil::checkIfStockDataOk(currentStockData, SettingManager::getInstance()->m_filterCondition[filterType], m_matchAll);
            }
        }

        if (currentStockData.m_beginTime > filterTime)
        {
            break;
        }

        if (ok)
        {
            hourStockDatas.append(currentStockData);
        }

        left++;
    }
}

FilterDataController::FilterDataController(QObject *parent)
    : QObject{parent}
{

}

void FilterDataController::run(const FilterParam& filterParam)
{
    if (m_name.isEmpty())
    {
        emit printLog(QString::fromWCharArray(L"过滤器名字为空"));
        emit runFinish();
        return;
    }

    bool delDetailDir = filterParam.m_notLossFilter || !filterParam.m_onlyFilterHour;
    if (delDetailDir)
    {
        QString savePath = QString::fromStdWString(CImPath::GetDataPath()) + m_name + QString::fromWCharArray(L"\\");
        if (!MergeDataController::removeDir(savePath))
        {
            emit printLog(QString::fromWCharArray(L"无法删除明细目录，请先关闭已打开的文件。"));
            emit runFinish();
            return;
        }
    }

    emit printLog(QString::fromWCharArray(L"开始筛选数据"));

    m_filterParam = filterParam;
    m_totalDays = filterParam.m_beginDate.daysTo(filterParam.m_endDate);
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
        QDate begin = filterParam.m_beginDate.addDays(i*dayPerThread);
        QDate end = filterParam.m_beginDate.addDays((i+1)*dayPerThread);
        if (i == threadCount-1)
        {
            end = filterParam.m_endDate;
        }

        DataFilter* dataFilter = new DataFilter();
        dataFilter->m_notLossFilter = filterParam.m_notLossFilter;
        dataFilter->m_notLossFilterDataType = filterParam.m_notLossFilterDataType;
        dataFilter->m_onlyFilterHour = filterParam.m_onlyFilterHour;
        dataFilter->m_onlyFilterToMonth = filterParam.m_onlyFilterToMonth;
        dataFilter->m_matchAll = filterParam.m_matchAll;
        dataFilter->m_beginDate = begin;
        dataFilter->m_endDate = end;

        QThread* thread = new QThread();
        dataFilter->moveToThread(thread);
        connect(thread, &QThread::started, dataFilter, &DataFilter::run);
        connect(thread, &QThread::finished, thread, &QThread::deleteLater);
        connect(dataFilter, &DataFilter::oneDayFinish, this, &FilterDataController::onFilterOneDayFinish);
        connect(dataFilter, &DataFilter::runFinish, this, &FilterDataController::onFilterRunFinish);
        m_dataFilters.append(dataFilter);
        m_dataFilterThreads.append(thread);
    }

    emit printLog(QString::fromWCharArray(L"开始筛选数据，使用线程数%1个").arg(threadCount));
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

void FilterDataController::onFilterRunFinish(DataFilter* dataFilter)
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
        if (m_stockDatas.isEmpty())
        {
            emit printLog(QString::fromWCharArray(L"筛选数据完成，没有数据。"));
        }
        else
        {
            emit printLog(QString::fromWCharArray(L"筛选数据完成"));
            if (m_filterParam.isOnlyFilterHourData())
            {
                saveStockHourDataSummaryInfo();
            }
            else
            {
                saveStockData();
                saveStockDataDetail();
            }
            QDesktopServices::openUrl(QUrl::fromLocalFile(QString::fromStdWString(CImPath::GetDataPath())));
        }

        emit runFinish();
    }
}

void FilterDataController::saveStockData()
{
    emit printLog(QString::fromWCharArray(L"开始保存数据"));

    // 按公历开始时间，股票名称排序
    std::sort(m_stockDatas.begin(), m_stockDatas.end(), [](const StockData& a, const StockData& b) {
        return a.m_beginTime < b.m_beginTime
                || (a.m_beginTime == b.m_beginTime && a.m_industryName < b.m_industryName)
                || (a.m_beginTime == b.m_beginTime && a.m_industryName == b.m_industryName && a.m_stockName < b.m_stockName);
    });

    QString result;
    QString lunarTime;
    qint64 beginTime = 0;
    QString industryName;
    QString stockName;
    bool hasIndustry = false;
    if (m_stockDatas.size() > 0 && !m_stockDatas[0].m_industryName.isEmpty())
    {
        hasIndustry = true;
    }

    for (const auto& stockData : m_stockDatas)
    {
        if (m_filterParam.isStockDataYearOrMonth())
        {
            // 只过滤到月，用农历时间
            if (!result.isEmpty() && stockData.m_lunarTime != lunarTime)
            {
                if (hasIndustry)
                {
                    result.append("\r\n\r\n");
                }
                else
                {
                    result.append("\r\n");
                }
            }
            if (stockData.m_lunarTime != lunarTime)
            {
                result += stockData.m_lunarTime + " ";
                lunarTime = stockData.m_lunarTime;
                industryName = "";
                stockName = "";
            }
        }
        else
        {
            // 周末不输出
            int dayOfWeek = QDateTime::fromSecsSinceEpoch(stockData.m_beginTime).date().dayOfWeek();
            if (dayOfWeek == 6 || dayOfWeek == 7)
            {
                continue;
            }

            // 不只过滤到月，用公历时间
            if (!result.isEmpty() && stockData.m_beginTime != beginTime)
            {
                if (hasIndustry)
                {
                    result.append("\r\n\r\n");
                }
                else
                {
                    result.append("\r\n");

                    // 跨周末加空行
                    static int sevenDay = 7*24*3600;
                    if (stockData.m_beginTime - beginTime >= sevenDay)
                    {
                        result.append("\r\n");
                    }
                    else
                    {
                        QDateTime now = QDateTime::fromSecsSinceEpoch(stockData.m_beginTime);
                        QDateTime last = QDateTime::fromSecsSinceEpoch(beginTime);
                        if (now.date().dayOfWeek() < last.date().dayOfWeek())
                        {
                            result.append("\r\n");
                        }
                    }
                }
            }

            if (stockData.m_beginTime != beginTime)
            {
                QString dateTimeStr = QDateTime::fromSecsSinceEpoch(stockData.m_beginTime).toString(QString::fromWCharArray(L"yyyy年M月d日"));
                result += dateTimeStr + " ";
                beginTime = stockData.m_beginTime;
                industryName = "";
                stockName = "";
            }
        }

        if (hasIndustry)
        {
            if (stockData.m_industryName != industryName)
            {
                if (!industryName.isEmpty())
                {
                    result += "\r\n                ";
                }
                stockName = "";
                industryName = stockData.m_industryName;
                result += industryName + "\r\n";
                if (m_filterParam.isStockDataYearOrMonth())
                {
                    result += "           ";
                }
                else
                {
                    result += "               ";
                }
            }
        }

        if (stockData.m_stockName != stockName)
        {
            // 时会有重复的股票，要去重
            result += stockData.m_stockName + " ";
            stockName = stockData.m_stockName;
        }
    }

    QString resultFilePath = QString::fromStdWString(CImPath::GetDataPath()) + m_name + QString::fromWCharArray(L".txt");
    QFile resultFile(resultFilePath);
    if (resultFile.open(QFile::WriteOnly))
    {
        resultFile.write(result.toUtf8());
        resultFile.close();        
    }
    else
    {
        qCritical("failed to open the result file");
    }    

    emit printLog(QString::fromWCharArray(L"保存数据完成"));    
}

void FilterDataController::saveStockDataDetail()
{
    if (m_stockDatas.isEmpty())
    {
        return;
    }

    emit printLog(QString::fromWCharArray(L"开始保存明细数据"));

    // 按行业、股票名称、时间排序
    std::sort(m_stockDatas.begin(), m_stockDatas.end(), [](const StockData& a, const StockData& b) {
        return a.m_industryName < b.m_industryName
                || (a.m_industryName == b.m_industryName && a.m_stockName < b.m_stockName)
                || (a.m_industryName == b.m_industryName && a.m_stockName == b.m_stockName && a.m_beginTime < b.m_beginTime);
    });

    QString industryName = m_stockDatas[0].m_industryName;
    QString stockName = m_stockDatas[0].m_stockName;
    int begin = 0;
    int end = 1;
    for (; end < m_stockDatas.size(); end++)
    {
        bool newStock = false;
        if (m_stockDatas[end].m_industryName != industryName
                || m_stockDatas[end].m_stockName != stockName)
        {
            newStock = true;
        }

        if (newStock)
        {
            saveStockDataDetail(begin, end);
            begin = end;
            industryName = m_stockDatas[end].m_industryName;
            stockName = m_stockDatas[end].m_stockName;
        }
    }

    // 最后一只股票
    saveStockDataDetail(begin, end);

    emit printLog(QString::fromWCharArray(L"保存明细数据完成"));
}

void FilterDataController::saveStockDataDetail(int begin, int end)
{
    if (begin >= end)
    {
        return;
    }

    QString savePath = QString::fromStdWString(CImPath::GetDataPath()) + m_name + QString::fromWCharArray(L"\\");
    QDir dir;
    if (!dir.exists(savePath))
    {
        dir.mkpath(savePath);
    }

    if (!m_stockDatas[begin].m_industryName.isEmpty())
    {
        savePath += m_stockDatas[begin].m_industryName + "\\";
        if (!dir.exists(savePath))
        {
            dir.mkpath(savePath);
        }
    }

    QString stockName = m_stockDatas[begin].m_stockName;
    QString result;
    QString lunarTime;
    qint64 beginTime = 0;
    for (int i=begin; i<end; i++)
    {
        const auto& stockData = m_stockDatas[i];
        if (m_filterParam.isStockDataYearOrMonth())
        {
            if (stockData.m_lunarTime == lunarTime)
            {
                continue;
            }

            // 输出时间，只过滤到月，用农历时间
            result += stockData.m_lunarTime + "\r\n";
            lunarTime = stockData.m_lunarTime;

            // 输出：年，宫内容
            appendSpaceChar(result, PREFIX_SPACE_COUNT);
            StockData yearStockData = stockData;
            if (stockData.m_lunarTime.indexOf(QString::fromWCharArray(L"月")) > 0)
            {
                yearStockData = findYearStockDataByMonth(stockData);
            }
            result += yearStockData.m_lunarTime;
            for (int fieldIndex=0; fieldIndex < DATA_FIELD_LENGTH; fieldIndex++)
            {
                appendSpaceChar(result, MIDDLE_SPACE_COUNT);
                result += yearStockData.m_data[fieldIndex];
            }
            result += "\r\n";

            // 输出：月，宫内容
            if (stockData.m_lunarTime.indexOf(QString::fromWCharArray(L"月")) > 0)
            {
                appendSpaceChar(result, PREFIX_SPACE_COUNT);
                appendSpaceChar(result, PREFIX_SPACE_COUNT);
                result += stockData.m_lunarTime;
                for (int fieldIndex=0; fieldIndex < DATA_FIELD_LENGTH; fieldIndex++)
                {
                    appendSpaceChar(result, MIDDLE_SPACE_COUNT);
                    result += stockData.m_data[fieldIndex];
                }
                result += "\r\n";
            }
        }
        else
        {
            if (stockData.m_beginTime == beginTime)
            {
                continue;
            }

            // 输出时间：日期
            QString dateTimeStr = QDateTime::fromSecsSinceEpoch(stockData.m_beginTime).toString(QString::fromWCharArray(L"yyyy年M月d日"));
            result += dateTimeStr + "\r\n";
            beginTime = stockData.m_beginTime;

            int prefixSpaceCount = PREFIX_SPACE_COUNT;

            // 输出：股票名称，月，宫内容
            appendSpaceChar(result, prefixSpaceCount);
            StockData monthStockData = findMonthStockDataByDay(stockData);
            result += monthStockData.m_lunarTime;
            for (int fieldIndex=0; fieldIndex < DATA_FIELD_LENGTH; fieldIndex++)
            {
                appendSpaceChar(result, MIDDLE_SPACE_COUNT);
                result += monthStockData.m_data[fieldIndex];
            }
            result += "\r\n";

            prefixSpaceCount += PREFIX_SPACE_COUNT;

            // 输出：上一日和时，宫内容
            StockData lastStockData = findLastDayStockDataByDay(stockData);
            if (!lastStockData.m_stockName.isEmpty())
            {
                appendDayStockData(prefixSpaceCount, result, lastStockData);
            }

            // 输出：当日和时，宫内容
            appendDayStockData(prefixSpaceCount, result, stockData);
        }
    }

    QString resultFilePath = savePath + stockName + QString::fromWCharArray(L"明细.txt");
    QFile resultFile(resultFilePath);
    if (resultFile.open(QFile::WriteOnly))
    {
        resultFile.write(result.toUtf8());
        resultFile.close();
    }
    else
    {
        qCritical("failed to open the result detail file");
    }
}

void FilterDataController::saveStockHourDataSummaryInfo()
{
    emit printLog(QString::fromWCharArray(L"开始保存数据"));

    std::sort(m_stockDatas.begin(), m_stockDatas.end(), [](const StockData& a, const StockData& b) {
        return a.m_beginTime < b.m_beginTime
                || (a.m_beginTime == b.m_beginTime && a.hourToInt() < b.hourToInt())
                || (a.m_beginTime == b.m_beginTime && a.hourToInt() == b.hourToInt() && a.m_stockName < b.m_stockName);
    });

    QString result; // 明细
    QString summaryResult; // 个数统计
    qint64 beginTime = 0;
    QString hour;
    QString stockName;
    int count = 0;
    for (const auto& stockData : m_stockDatas)
    {
        // 周末不输出
        int dayOfWeek = QDateTime::fromSecsSinceEpoch(stockData.m_beginTime).date().dayOfWeek();
        if (dayOfWeek == 6 || dayOfWeek == 7)
        {
            continue;
        }

        // 新的一天，要换行
        if (stockData.m_beginTime != beginTime)
        {
            if (!result.isEmpty())
            {
                result.append("\r\n\r\n");
            }

            if (!summaryResult.isEmpty())
            {
                summaryResult += QString::number(count);
                summaryResult.append("\r\n\r\n");
            }

            QString dateTimeStr = QDateTime::fromSecsSinceEpoch(stockData.m_beginTime).toString(QString::fromWCharArray(L"yyyy年M月d日"));
            result += dateTimeStr + "\r\n";
            summaryResult += dateTimeStr + "\r\n";
            beginTime = stockData.m_beginTime;
            hour = "";
            stockName = "";
            count = 0;
        }

        if (stockData.m_hour != hour)
        {
            if (!hour.isEmpty())
            {
                result += "\r\n";
                summaryResult += QString::number(count) + "\r\n";
            }

            appendSpaceChar(result, 14);
            result += stockData.m_hour + ": ";

            appendSpaceChar(summaryResult, 14);
            summaryResult += stockData.m_hour + ": ";

            hour = stockData.m_hour;
            stockName = "";
            count = 0;
        }

        if (stockData.m_stockName != stockName)
        {
            result += stockData.m_stockName + " ";
            count += 1;
            stockName = stockData.m_stockName;
        }
    }

    if (!hour.isEmpty())
    {
        summaryResult += QString::number(count);
    }

    QString resultFilePath = QString::fromStdWString(CImPath::GetDataPath()) + QString::fromWCharArray(L"巳午未明细.txt");
    QFile resultFile(resultFilePath);
    if (resultFile.open(QFile::WriteOnly))
    {
        resultFile.write(result.toUtf8());
        resultFile.close();
    }
    else
    {
        qCritical("failed to open the yi wu wei detail file");
    }

    QString summaryFilePath = QString::fromStdWString(CImPath::GetDataPath()) + QString::fromWCharArray(L"巳午未数字统计.txt");
    QFile summaryFile(summaryFilePath);
    if (summaryFile.open(QFile::WriteOnly))
    {
        summaryFile.write(summaryResult.toUtf8());
        summaryFile.close();
    }
    else
    {
        qCritical("failed to open the summary result file");
    }

    emit printLog(QString::fromWCharArray(L"保存数据完成"));
}

void FilterDataController::appendSpaceChar(QString& result, int count)
{
    for (int i=0; i<count; i++)
    {
        result.append(' ');
    }
}

StockData FilterDataController::findYearStockDataByMonth(const StockData& monthStockData)
{
    // 从前1年开始(考虑到农历公历时间差)，找到开始筛选的数据点
    QDateTime monthBeginDateTime = QDateTime::fromSecsSinceEpoch(monthStockData.m_beginTime);
    QDateTime beginSearchDateTime = monthBeginDateTime.addYears(-1);
    qint64 beginSearchTime = beginSearchDateTime.toSecsSinceEpoch();

    const QVector<StockData>& stockDatas = DataManager::getInstance()->m_stockDatas[STOCK_DATA_YEAR];
    int left = DataFilter::findIndex(stockDatas, beginSearchTime);

    // 开始筛选
    while (left < stockDatas.length())
    {
        const StockData& currentStockData = stockDatas[left];
        if (currentStockData.m_stockName == monthStockData.m_stockName
                && monthStockData.m_lunarTime.indexOf(currentStockData.m_lunarTime) == 0)
        {
            return currentStockData;
        }

        QDateTime beginDateTime = QDateTime::fromSecsSinceEpoch(currentStockData.m_beginTime);
        if (beginDateTime.date().year() - monthBeginDateTime.date().year() >= 2)
        {
            break;
        }

        left++;
    }

    qCritical("failed to find the year stock data of %s %s",
              monthStockData.m_lunarTime.toStdString().c_str(),
              monthStockData.m_stockName.toStdString().c_str());
    return StockData();
}

StockData FilterDataController::findMonthStockDataByDay(const StockData& dayStockData)
{
    // 从前3个月开始(考虑到农历公历时间差)，找到开始筛选的数据点
    QDate date = QDateTime::fromSecsSinceEpoch(dayStockData.m_beginTime).date();
    QDateTime beginSearchDateTime;
    QDate tempDate = date.addMonths(-3);
    beginSearchDateTime.setDate(QDate(tempDate.year(), tempDate.month(), 1));
    qint64 beginSearchTime = beginSearchDateTime.toSecsSinceEpoch();

    const QVector<StockData>& stockDatas = DataManager::getInstance()->m_stockDatas[STOCK_DATA_MONTH];
    int left = DataFilter::findIndex(stockDatas, beginSearchTime);
    while (left < stockDatas.length())
    {
        const StockData& currentStockData = stockDatas[left];
        if (dayStockData.m_beginTime >= currentStockData.m_beginTime
                && dayStockData.m_beginTime <= currentStockData.m_endTime
                && currentStockData.m_stockName == dayStockData.m_stockName)
        {
            return currentStockData;
        }

        if (currentStockData.m_beginTime > dayStockData.m_beginTime)
        {
            break;
        }

        left++;
    }

    qCritical("failed to find the month stock data of %s %d-%d",
              dayStockData.m_stockName.toStdString().c_str(),
              date.year(), date.month());
    return StockData();
}

StockData FilterDataController::findLastDayStockDataByDay(const StockData& dayStockData)
{
    QDate date = QDateTime::fromSecsSinceEpoch(dayStockData.m_beginTime).date();
    QDateTime beginSearchDateTime;
    beginSearchDateTime.setDate(date.addDays(-1));
    qint64 beginSearchTime = beginSearchDateTime.toSecsSinceEpoch();

    const QVector<StockData>& stockDatas = DataManager::getInstance()->m_stockDatas[STOCK_DATA_DAY];
    int left = DataFilter::findIndex(stockDatas, beginSearchTime);
    qint64 filterTime = beginSearchTime;
    while (left < stockDatas.length())
    {
        const StockData& currentStockData = stockDatas[left];
        if (currentStockData.m_beginTime == filterTime
                && currentStockData.m_stockName == dayStockData.m_stockName)
        {
            return currentStockData;
        }

        if (currentStockData.m_beginTime > filterTime)
        {
            break;
        }

        left++;
    }

    return StockData();
}

void FilterDataController::appendDayStockData(int prefixSpaceCount, QString& result, const StockData& stockData)
{
    // 输出日的数据
    qint64 beginSearchTime = stockData.m_beginTime;
    const QVector<StockData>& stockDatas = DataManager::getInstance()->m_stockDatas[STOCK_DATA_DAY];
    int left = DataFilter::findIndex(stockDatas, beginSearchTime);
    qint64 filterTime = beginSearchTime;
    while (left < stockDatas.length())
    {
        const StockData& currentStockData = stockDatas[left];
        if (currentStockData.m_beginTime == filterTime
                && currentStockData.m_stockName == stockData.m_stockName)
        {
            appendSpaceChar(result, prefixSpaceCount);
            QString dateTimeStr = QDateTime::fromSecsSinceEpoch(currentStockData.m_beginTime).toString(QString::fromWCharArray(L"yyyy年M月d日"));
            result += dateTimeStr;
            for (int i=0; i<DATA_FIELD_LENGTH; i++)
            {
                appendSpaceChar(result, MIDDLE_SPACE_COUNT);
                result += currentStockData.m_data[i];
            }
            result += "\r\n";
            break;
        }

        if (currentStockData.m_beginTime > filterTime)
        {
            break;
        }

        left++;
    }

    prefixSpaceCount += PREFIX_SPACE_COUNT;

    // 输出已午未的数据
    for (int i=STOCK_DATA_HOUR_YI; i<=STOCK_DATA_HOUR_WEI; i++)
    {
        const QVector<StockData>& stockDatas = DataManager::getInstance()->m_stockDatas[i];
        left = DataFilter::findIndex(stockDatas, beginSearchTime);
        qint64 filterTime = beginSearchTime;
        while (left < stockDatas.length())
        {
            const StockData& currentStockData = stockDatas[left];
            if (currentStockData.m_beginTime == filterTime
                    && currentStockData.m_stockName == stockData.m_stockName)
            {
                appendSpaceChar(result, prefixSpaceCount);
                result += currentStockData.m_hour;
                for (int i=0; i<DATA_FIELD_LENGTH; i++)
                {
                    appendSpaceChar(result, MIDDLE_SPACE_COUNT);
                    result += currentStockData.m_data[i];
                }
                result += "\r\n";
                break;
            }

            if (currentStockData.m_beginTime > filterTime)
            {
                break;
            }

            left++;
        }
    }
}
