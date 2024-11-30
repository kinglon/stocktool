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
        emit oneDayFinish();
    }
    emit runFinish(this);
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
        filterYearData(date, true, yearStockDatas);
    }

    if (yearStockDatas.empty())
    {
        return;
    }    

    // 筛选月数据
    QVector<StockData> monthStockDatas;
    filterMonthData(date, true, yearStockDatas, monthStockDatas);
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
        filterYearData(date, false, yearStockDatas);
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
        filterMonthData(date, false, yearStockDatas, monthStockDatas);
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
        filterDayData(date, monthStockDatas, dayStockDatas);
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

void DataFilter::filterYearData(QDate date, bool useLunarTime, QVector<StockData>& yearStockDatas)
{
    // 从前1年开始(考虑到农历公历时间差)，找到开始筛选的数据点
    QDateTime beginSearchDateTime;
    beginSearchDateTime.setDate(QDate(date.year()-1, 1, 1));
    qint64 beginSearchTime = beginSearchDateTime.toSecsSinceEpoch();

    const QVector<StockData>& stockDatas = DataManager::getInstance()->m_stockDatas[STOCK_DATA_YEAR];
    int left = findIndex(stockDatas, beginSearchTime);

    // 开始筛选
    FilterCondition yearFilterCondition = SettingManager::getInstance()->m_filterCondition[YEAR_FILTER_CONDTION];
    QString year = date.toString("yyyy");
    while (left < stockDatas.length())
    {
        const StockData& currentStockData = stockDatas[left];
        if (useLunarTime)
        {
            if (currentStockData.m_lunarTime == year)
            {
                if (checkIfStockDataOk(currentStockData, yearFilterCondition))
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
                if (checkIfStockDataOk(currentStockData, yearFilterCondition))
                {
                    yearStockDatas.append(currentStockData);
                }
            }

            if (beginDateTime.date().year() > date.year())
            {
                break;
            }
        }

        left++;
    }
}

void DataFilter::filterMonthData(QDate date, bool useLunarTime, const QVector<StockData>& yearStockDatas, QVector<StockData>& monthStockDatas)
{
    // 从前3个月开始(考虑到农历公历时间差)，找到开始筛选的数据点
    QDateTime beginSearchDateTime;
    QDate tempDate = date.addMonths(-3);
    beginSearchDateTime.setDate(QDate(tempDate.year(), tempDate.month(), 1));
    qint64 beginSearchTime = beginSearchDateTime.toSecsSinceEpoch();

    const QVector<StockData>& stockDatas = DataManager::getInstance()->m_stockDatas[STOCK_DATA_MONTH];
    int left = findIndex(stockDatas, beginSearchTime);

    // 开始筛选
    FilterCondition monthFilterCondition = SettingManager::getInstance()->m_filterCondition[MONTH_FILTER_CONDTION];
    QString month = date.toString(QString::fromWCharArray(L"yyyy年M月"));
    QDateTime filterDateTime;
    filterDateTime.setDate(QDate(date.year(), date.month(), 1));
    qint64 filterTime = filterDateTime.toSecsSinceEpoch();
    while (left < stockDatas.length())
    {
        const StockData& currentStockData = stockDatas[left];
        bool ok = false;
        if (useLunarTime)
        {
            if (currentStockData.m_lunarTime == month)
            {
                if (checkIfStockDataOk(currentStockData, monthFilterCondition))
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
                if (checkIfStockDataOk(currentStockData, monthFilterCondition))
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

        left++;
    }
}

void DataFilter::filterDayData(QDate date, const QVector<StockData>& monthStockDatas, QVector<StockData>& dayStockDatas)
{
    QDateTime beginSearchDateTime;
    beginSearchDateTime.setDate(date);
    qint64 beginSearchTime = beginSearchDateTime.toSecsSinceEpoch();

    const QVector<StockData>& stockDatas = DataManager::getInstance()->m_stockDatas[STOCK_DATA_DAY];
    int left = findIndex(stockDatas, beginSearchTime);

    // 开始筛选
    FilterCondition dayFilterCondition = SettingManager::getInstance()->m_filterCondition[DAY_FILTER_CONDTION];
    qint64 filterTime = beginSearchTime;
    while (left < stockDatas.length())
    {
        const StockData& currentStockData = stockDatas[left];
        bool ok = false;
        if (currentStockData.m_beginTime == filterTime)
        {
            if (checkIfStockDataOk(currentStockData, dayFilterCondition))
            {
                ok = true;
            }
        }

        if (currentStockData.m_beginTime > filterTime)
        {
            break;
        }

        if (ok)
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
            if (checkIfStockDataOk(currentStockData, filterCondition))
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
            if (checkIfStockDataOk(currentStockData, SettingManager::getInstance()->m_filterCondition[filterType]))
            {
                ok = true;
            }
        }

        if (currentStockData.m_beginTime > filterTime)
        {
            break;
        }

        if (ok)
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

        left++;
    }
}

bool DataFilter::checkIfStockDataOk(StockData stockData, const FilterCondition& filterCondition)
{
    if (!filterCondition.isEnable())
    {
        return true;
    }

    QString data1 = stockData.m_data[0];
    QString data2 = stockData.m_data[1];

    // 有空字先借位    
    if (stockData.m_data[0].indexOf(QString::fromWCharArray(L"空")) >= 0)
    {
        stockData.m_data[0] += data2;
    }
    if (stockData.m_data[1].indexOf(QString::fromWCharArray(L"空")) >= 0)
    {
        stockData.m_data[1] += data1;
    }

    // 删除：阳忌、昌科、曲科、阳（忌）、昌（科）、曲（科）
    for (int i=0; i<4; i++)
    {
        stockData.m_data[i].replace(QString::fromWCharArray(L"阳忌"), "");
        stockData.m_data[i].replace(QString::fromWCharArray(L"昌科"), "");
        stockData.m_data[i].replace(QString::fromWCharArray(L"曲科"), "");
        stockData.m_data[i].replace(QString::fromWCharArray(L"阳(忌)"), "");
        stockData.m_data[i].replace(QString::fromWCharArray(L"昌(科)"), "");
        stockData.m_data[i].replace(QString::fromWCharArray(L"曲(科)"), "");
    }

    // 一二宫有存字，就删除禄字
    if (data1.indexOf(QString::fromWCharArray(L"存")) >= 0 ||
            data2.indexOf(QString::fromWCharArray(L"存")) >= 0)
    {
        stockData.m_data[0].replace(QString::fromWCharArray(L"禄"), "");
        stockData.m_data[1].replace(QString::fromWCharArray(L"禄"), "");
    }

    // 一二宫有禄字，就删除存字
    if (data1.indexOf(QString::fromWCharArray(L"禄")) >= 0 ||
            data2.indexOf(QString::fromWCharArray(L"禄")) >= 0)
    {
        stockData.m_data[0].replace(QString::fromWCharArray(L"存"), "");
        stockData.m_data[1].replace(QString::fromWCharArray(L"存"), "");
    }

    // 检查一宫不含，每个字都要满足
    for (int i=0; i<filterCondition.m_oneExclude.length(); i++)
    {
        QString word = filterCondition.m_oneExclude[i];
        if (word == QString::fromWCharArray(L"存"))
        {
            if (hasCunWord(stockData.m_data[0], stockData.m_data[0], stockData.m_data[1]))
            {
                return false;
            }
        }
        else
        {
            if (stockData.m_data[0].indexOf(word) >= 0 && haveWordWithoutKuohao(word, stockData.m_data))
            {
                return false;
            }
        }
    }

    // 检查二宫不含，每个字都要满足
    for (int i=0; i<filterCondition.m_twoExclude.length(); i++)
    {
        QString word = filterCondition.m_twoExclude[i];
        if (word == QString::fromWCharArray(L"存"))
        {
            if (hasCunWord(stockData.m_data[1], stockData.m_data[0], stockData.m_data[1]))
            {
                return false;
            }
        }
        else
        {
            if (stockData.m_data[1].indexOf(word) >= 0 && haveWordWithoutKuohao(word, stockData.m_data))
            {
                return false;
            }
        }
    }

    // 检查一宫含，只要一个满足
    bool ok = false;
    for (int i=0; i<filterCondition.m_oneInclude.length(); i++)
    {
        QString word = filterCondition.m_oneInclude[i];
        if (word == QString::fromWCharArray(L"存"))
        {
            if (hasCunWord(stockData.m_data[0], stockData.m_data[0], stockData.m_data[1]))
            {
                ok = true;
                break;
            }
        }
        else
        {
            if (stockData.m_data[0].indexOf(word) >= 0 && haveWordWithoutKuohao(word, stockData.m_data))
            {
                ok = true;
                break;
            }
        }
    }
    if (filterCondition.m_oneInclude.length() > 0 && !ok)
    {
        return false;
    }

    // 检查二宫含，只要一个满足
    ok = false;
    for (int i=0; i<filterCondition.m_twoInclude.length(); i++)
    {
        QString word = filterCondition.m_twoInclude[i];
        if (word == QString::fromWCharArray(L"存"))
        {
            if (hasCunWord(stockData.m_data[1], stockData.m_data[0], stockData.m_data[1]))
            {
                ok = true;
                break;
            }
        }
        else
        {
            if (stockData.m_data[1].indexOf(word) >= 0 && haveWordWithoutKuohao(word, stockData.m_data))
            {
                ok = true;
                break;
            }
        }
    }
    if (filterCondition.m_twoInclude.length() > 0 && !ok)
    {
        return false;
    }

    return true;
}

bool DataFilter::hasCunWord(QString data, QString data1, QString data2)
{
    QString cun = QString::fromWCharArray(L"存");

    // 只能有一个存字
    int count = 0;
    for (int i=0; i<data.length(); i++)
    {
        if (data[i] == cun)
        {
            count++;
        }
    }
    if (count != 1)
    {
        return false;
    }

    // 存前后是括号，返回false
    if (data.indexOf(QString::fromWCharArray(L"(存)")) >= 0)
    {
        return false;
    }

    // 一二宫带有禄字，返回false
    if (data1.indexOf(QString::fromWCharArray(L"禄")) >= 0 ||
            data2.indexOf(QString::fromWCharArray(L"禄")) >= 0)
    {
        return false;
    }

    return true;
}

bool DataFilter::haveWordWithoutKuohao(QString word, QString data[4])
{
    for (int j=0; j<4; j++)
    {
        QString currentData = data[j];
        for (int i=0; i<currentData.length(); i++)
        {
            if (currentData[i] == word)
            {
                if (i == 0 || currentData[i-1] != "(" || i == currentData.length() - 1
                        || currentData[i+1] != ")")
                {
                    return true;
                }
            }
        }
    }

    return false;
}

FilterDataController::FilterDataController(QObject *parent)
    : QObject{parent}
{

}

void FilterDataController::run(bool onlyFilterToMonth, QDate beginDate, QDate endDate)
{
    QString savePath = QString::fromStdWString(CImPath::GetDataPath()) + QString::fromWCharArray(L"明细\\");
    if (!MergeDataController::removeDir(savePath))
    {
        emit printLog(QString::fromWCharArray(L"无法删除明细目录，请先关闭已打开的文件。"));
        emit runFinish();
        return;
    }

    emit printLog(QString::fromWCharArray(L"开始筛选数据"));

    m_onlyFilterToMonth = onlyFilterToMonth;
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

        DataFilter* dataFilter = new DataFilter();
        dataFilter->m_onlyFilterToMonth = onlyFilterToMonth;
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
        emit printLog(QString::fromWCharArray(L"筛选数据完成，共%1条").arg(m_stockDatas.size()));
        if (!m_stockDatas.isEmpty())
        {
            saveStockData();
            saveStockDataDetail();
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
        if (m_onlyFilterToMonth)
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
                if (m_onlyFilterToMonth)
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

    QString resultFilePath = QString::fromStdWString(CImPath::GetDataPath()) + QString::fromWCharArray(L"结果.txt");
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

    QString savePath = QString::fromStdWString(CImPath::GetDataPath()) + QString::fromWCharArray(L"明细\\");
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
        if (m_onlyFilterToMonth)
        {
            if (stockData.m_lunarTime == lunarTime)
            {
                continue;
            }

            // 输出时间，只过滤到月，用农历时间
            result += stockData.m_lunarTime + "\r\n";
            lunarTime = stockData.m_lunarTime;

            // 输出：年，4宫内容
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

            // 输出：月，4宫内容
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

            // 输出：股票名称，月，4宫内容
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

            // 输出：上一日和时，4宫内容
            StockData lastStockData = findLastDayStockDataByDay(stockData);
            if (!lastStockData.m_stockName.isEmpty())
            {
                appendDayStockData(prefixSpaceCount, result, lastStockData);
            }

            // 输出：当日和时，4宫内容
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
