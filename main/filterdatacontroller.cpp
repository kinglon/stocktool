#include "filterdatacontroller.h"
#include <QTimer>
#include <QFile>
#include "Utility/ImPath.h"
#include "settingmanager.h"
#include "datamanager.h"

DataFilter::DataFilter(QObject *parent)
    : QObject{parent}
{

}

void DataFilter::run()
{
    for (QDate date=m_beginDate; date < m_endDate; date = date.addDays(1))
    {
        if (m_onlyFilterToMonth)
        {
            filterOnlyToMonth(date);
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
        yearStockDatas = DataManager::getInstance()->m_stockDatas[YEAR_FILTER_CONDTION];
    }
    else
    {
        filterYearData(date, true, yearStockDatas);
    }

    if (yearStockDatas.empty())
    {
        return;
    }

    if (!SettingManager::getInstance()->m_filterCondition[MONTH_FILTER_CONDTION].isEnable())
    {
        m_stockDatas.append(yearStockDatas);
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
        yearStockDatas = DataManager::getInstance()->m_stockDatas[YEAR_FILTER_CONDTION];
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

    // 筛选时的数据
    if (!SettingManager::getInstance()->m_filterCondition[YI_HOUR_FILTER_CONDTION].isEnable() &&
            !SettingManager::getInstance()->m_filterCondition[WU_HOUR_FILTER_CONDTION].isEnable() &&
            !SettingManager::getInstance()->m_filterCondition[WEI_HOUR_FILTER_CONDTION].isEnable())
    {
        m_stockDatas.append(secondDayStockDatas);
    }
    else
    {
        QVector<StockData> hourDayStockDatas;
        filterHourData(date, secondDayStockDatas, hourDayStockDatas);
        m_stockDatas.append(hourDayStockDatas);
    }
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
    QString month = date.toString("yyyy年M月");
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

void DataFilter::filterHourData(QDate date, const QVector<StockData>& dayStockDatas, QVector<StockData>& hourStockDatas)
{
    QDateTime beginSearchDateTime;
    beginSearchDateTime.setDate(date);
    qint64 beginSearchTime = beginSearchDateTime.toSecsSinceEpoch();

    const QVector<StockData>& stockDatas = DataManager::getInstance()->m_stockDatas[STOCK_DATA_HOUR];
    int left = findIndex(stockDatas, beginSearchTime);

    // 开始筛选
    qint64 filterTime = beginSearchTime;
    while (left < stockDatas.length())
    {
        const StockData& currentStockData = stockDatas[left];
        bool ok = false;
        if (currentStockData.m_beginTime == filterTime)
        {
            if (currentStockData.m_hour == QString::fromWCharArray(L"已"))
            {
                if (checkIfStockDataOk(currentStockData, SettingManager::getInstance()->m_filterCondition[YI_HOUR_FILTER_CONDTION]))
                {
                    ok = true;
                }
            }
            else if (currentStockData.m_hour == QString::fromWCharArray(L"午"))
            {
                if (checkIfStockDataOk(currentStockData, SettingManager::getInstance()->m_filterCondition[WU_HOUR_FILTER_CONDTION]))
                {
                    ok = true;
                }
            }
            else if (currentStockData.m_hour == QString::fromWCharArray(L"未"))
            {
                if (checkIfStockDataOk(currentStockData, SettingManager::getInstance()->m_filterCondition[WEI_HOUR_FILTER_CONDTION]))
                {
                    ok = true;
                }
            }
        }

        if (currentStockData.m_beginTime > filterTime)
        {
            break;
        }

        if (ok)
        {
            for (const auto& stockData : dayStockDatas)
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
    // 有空字先借位
    QString data1 = stockData.m_data[0];
    QString data2 = stockData.m_data[1];
    if (stockData.m_data[0].indexOf(QString::fromWCharArray(L"空") >= 0))
    {
        stockData.m_data[0] += data2;
    }
    if (stockData.m_data[1].indexOf(QString::fromWCharArray(L"空") >= 0))
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
    if (stockData.m_data[0].indexOf(QString::fromWCharArray(L"存")) >= 0 ||
            stockData.m_data[1].indexOf(QString::fromWCharArray(L"存")) >= 0)
    {
        stockData.m_data[0].replace(QString::fromWCharArray(L"禄"), "");
        stockData.m_data[1].replace(QString::fromWCharArray(L"禄"), "");
    }

    // 检查一宫不含
    for (int i=0; i<filterCondition.m_oneExclude.length(); i++)
    {
        if (stockData.m_data[0].indexOf(filterCondition.m_oneExclude[i]) >= 0)
        {
            return false;
        }
    }

    // 检查二宫不含
    for (int i=0; i<filterCondition.m_twoExclude.length(); i++)
    {
        if (stockData.m_data[1].indexOf(filterCondition.m_twoExclude[i]) >= 0)
        {
            return false;
        }
    }

    // 检查一宫含
    for (int i=0; i<filterCondition.m_oneInclude; i++)
    {
        QString word = filterCondition.m_oneInclude[i];
        if (word == QString::fromWCharArray(L"存"))
        {
            if (!hasCunWord(stockData.m_data[0], stockData.m_data[0], stockData.m_data[1]))
            {
                return false;
            }
        }
        else
        {
            if (stockData.m_data[0].indexOf(word) < 0)
            {
                return false;
            }

            // 如果只是有前后括号，返回false
            if (!haveWordWithoutKuohao(word, stockData.m_data))
            {
                return false;
            }
        }
    }

    // 检查二宫含
    for (int i=0; i<filterCondition.m_twoInclude.length(); i++)
    {
        QString word = filterCondition.m_twoInclude[i];
        if (word == QString::fromWCharArray(L"存"))
        {
            if (!hasCunWord(stockData.m_data[1], stockData.m_data[0], stockData.m_data[1]))
            {
                return false;
            }
        }
        else
        {
            if (stockData.m_data[1].indexOf(word) < 0)
            {
                return false;
            }

            // 如果只是有前后括号，返回false
            if (!haveWordWithoutKuohao(word, stockData.m_data))
            {
                return false;
            }
        }
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
        saveStockData();
    }
}

void FilterDataController::saveStockData()
{
    emit printLog(QString::fromWCharArray(L"开始保存数据"));

    QString result;
    if (m_onlyFilterToMonth)
    {
        // 按农历排序
        std::sort(m_stockDatas.begin(), m_stockDatas.end(), [](const StockData& a, const StockData& b) {
            QDateTime aDateTime;
            if (a.m_lunarTime.indexOf(QString::fromWCharArray(L"年")) > 0)
            {
                aDateTime = QDateTime::fromString(a.m_lunarTime, "yyyy年M月");
            }
            else
            {
                aDateTime = QDateTime::fromString(a.m_lunarTime, "yyyy");
            }

            QDateTime bDateTime;
            if (b.m_lunarTime.indexOf(QString::fromWCharArray(L"年")) > 0)
            {
                bDateTime = QDateTime::fromString(b.m_lunarTime, "yyyy年M月");
            }
            else
            {
                bDateTime = QDateTime::fromString(b.m_lunarTime, "yyyy");
            }

            return aDateTime < bDateTime;
        });

        // 每行: 时间 股票1 股票2
        QString lunarTime;
        for (const auto& stockData : m_stockDatas)
        {
            if (!result.isEmpty() && stockData.m_lunarTime != lunarTime)
            {
                result.append("\r\n");
            }
            if (stockData.m_lunarTime != lunarTime)
            {
                result += stockData.m_lunarTime + " ";
                lunarTime = stockData.m_lunarTime;
            }
            result += stockData.m_stockName + " ";
        }
    }
    else
    {
        // 按公历开始时间，股票名称排序
        std::sort(m_stockDatas.begin(), m_stockDatas.end(), [](const StockData& a, const StockData& b) {
            return a.m_beginTime < b.m_beginTime || (a.m_beginTime == b.m_beginTime && a.m_stockName < b.m_stockName);
        });

        // 每行: 时间 股票1 股票2
        qint64 beginTime = 0;
        QString stockName;
        for (const auto& stockData : m_stockDatas)
        {
            if (!result.isEmpty() && stockData.m_beginTime != beginTime)
            {
                result.append("\r\n");
            }
            if (stockData.m_beginTime != beginTime)
            {
                QString dateTimeStr = QDateTime::fromSecsSinceEpoch(stockData.m_beginTime).toString(QString::fromWCharArray(L"yyyy年M月d日"));
                result += dateTimeStr + " ";
                beginTime = stockData.m_beginTime;
                stockName = "";
            }
            if (stockData.m_stockName != stockName)
            {
                // 时会有重复的股票，要去重
                result += stockData.m_stockName + " ";
                stockName = stockData.m_stockName;
            }
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
    emit runFinish();
}
