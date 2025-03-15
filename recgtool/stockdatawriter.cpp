#include "stockdatawriter.h"
#include "settingmanager.h"
#include <QtMath>
#include <QDateTime>
#include <QSet>
#include <QDir>

// 分割空格
#define SPLIT_SPACE "  "

#define NEW_LINE "\r\n"

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof(arr[0]))

StockDataWriter::StockDataWriter()
{

}

QString StockDataWriter::getResult(QVector<StockData>& stockDatas)
{
    // 按值算法，计算股票的得分
    QVector<StockScore> stockScores = calcStockScore(stockDatas);

    // 按得分、行业名称、股票名称、开始时间排序
    std::sort(stockDatas.begin(), stockDatas.end(), [&stockScores](const StockData& a, const StockData& b) {
        float aScore = 0.0f;
        float bScore = 0.0f;
        for (const auto& stockScore : stockScores)
        {
            if (stockScore.m_industry == a.m_industryName && stockScore.m_stockName == a.m_stockName)
            {
                aScore = stockScore.getAvarageScore();
                break;
            }
        }

        for (const auto& stockScore : stockScores)
        {
            if (stockScore.m_industry == b.m_industryName && stockScore.m_stockName == b.m_stockName)
            {
                bScore = stockScore.getAvarageScore();
                break;
            }
        }

        if (qFabs(aScore - bScore) < 1e-5)
        {
            return a.m_industryName < b.m_industryName
                    || (a.m_industryName == b.m_industryName && a.m_stockName < b.m_stockName)
                    || (a.m_industryName == b.m_industryName && a.m_stockName == b.m_stockName && a.m_beginTime < b.m_beginTime);
        }

        if (SettingManager::getInstance()->m_sortDesc)
        {
            return aScore > bScore;
        }
        else
        {
            return aScore < bScore;
        }
    });

    // 按要求输出格式输出内容
    QString result;
    QString currentIndusty;
    QString currentStockName;
    int currentYear = 0;
    int currentMonth = 0;
    for (const auto& stockData : stockDatas)
    {
        if (stockData.m_industryName != currentIndusty || stockData.m_stockName != currentStockName)
        {
            // 新股票
            writeStockXianContent(stockData, result);
            currentIndusty = stockData.m_industryName;
            currentStockName = stockData.m_stockName;
            currentYear = 0;
            currentMonth = 0;
        }

        QDateTime dateTime = QDateTime::fromSecsSinceEpoch(stockData.m_beginTime);
        if (dateTime.date().year() != currentYear)
        {
            // 新一年
            writeStockYearContent(stockData, result);
            currentYear = dateTime.date().year();
            currentMonth = 0;
        }

        if (dateTime.date().month() != currentMonth)
        {
            // 新一月
            writeStockMonthContent(stockData, result);
            currentMonth = dateTime.date().month();
        }

        writeStockDayContent(stockData, result);
    }

    return result;
}

bool StockDataWriter::saveResult(const QString& savePath)
{
    const QString& result = StockDataManager::getInstance()->m_result;
    if (result.isEmpty())
    {
        return false;
    }

    // 获取文件名前缀，如果只有一支股票，就用股票名称，否则用股票所在的文件夹名称
    QString stockName;
    for (int i=STOCK_TYPE_GEGU; i<=STOCK_TYPE_ZHISHU; i++)
    {
        if (!SettingManager::getInstance()->m_stockSetting[i].m_enable)
        {
            continue;
        }

        // 根据限数据统计股票数
        QSet<QString> stockNames;
        const QVector<StockData>& xianDatas = StockDataManager::getInstance()->m_dataManagers[i]->m_stockDatas[STOCK_DATA_XIAN];
        for (const StockData& xianData : xianDatas)
        {
            stockNames.insert(xianData.m_stockName);
        }

        if (stockNames.empty())
        {
            continue;
        }

        if (stockNames.size() == 1)
        {
            stockName = *stockNames.begin();
        }
        else
        {
            QDir dir(SettingManager::getInstance()->m_stockSetting[i].m_stockPath);
            stockName = dir.dirName();
        }

        break;
    }

    if (stockName.isEmpty())
    {
        return false;
    }

    // 组装文件名
    QString beginDate = QDateTime::fromSecsSinceEpoch(SettingManager::getInstance()->m_recgDateStart).toString(QString::fromWCharArray(L"yyyy年M月d日"));
    QString endDate = QDateTime::fromSecsSinceEpoch(SettingManager::getInstance()->m_recgDateEnd).toString(QString::fromWCharArray(L"yyyy年M月d日"));
    QString fileName = QString::fromWCharArray(L"%1识别日%2-%3.txt").arg(
                stockName, beginDate, endDate);

    // 保存到文件
    QFile resultFile(savePath + "\\" +fileName);
    if (resultFile.open(QFile::WriteOnly))
    {
        resultFile.write(result.toUtf8());
        resultFile.close();
        return true;
    }
    else
    {
        return false;
    }
}

QVector<StockScore> StockDataWriter::calcStockScore(const QVector<StockData>& stockDatas)
{
    QVector<StockScore> stockScores;
    for (const auto& stockData : stockDatas)
    {
        // 查找个股涨跌
        float geGuZhangDie = 10.0f;
        if (SettingManager::getInstance()->m_stockSetting[STOCK_TYPE_GEGU].m_enable)
        {
            QVector<DayLineData>& dayLineDatas = StockDataManager::getInstance()->m_dataManagers[STOCK_TYPE_GEGU]->m_dayLineDatas[STOCK_DATA_DAY];
            for (const auto& dayLineData : dayLineDatas)
            {
                // 个股会有多支股票，需要匹配股票和日期
                if (stockData.m_industryName == dayLineData.m_industryName
                        && stockData.m_stockName == dayLineData.m_stockName
                        && stockData.m_beginTime == dayLineData.m_beginTime)
                {
                    geGuZhangDie = dayLineData.m_zhangFu;
                    break;
                }
            }
        }

        // 查找概念2涨跌
        float gaiNian2ZhangDie = 0.0f;
        if (SettingManager::getInstance()->m_stockSetting[STOCK_TYPE_GAINIAN2].m_enable)
        {
            QVector<DayLineData>& dayLineDatas = StockDataManager::getInstance()->m_dataManagers[STOCK_TYPE_GAINIAN2]->m_dayLineDatas[STOCK_DATA_DAY];
            for (const auto& dayLineData : dayLineDatas)
            {
                // 只有一支股票，只需匹配日期
                if (stockData.m_beginTime == dayLineData.m_beginTime)
                {
                    gaiNian2ZhangDie = dayLineData.m_zhangFu;
                    break;
                }
            }
        }

        // 查找概念1涨跌
        float gaiNian1ZhangDie = 0.0f;
        if (SettingManager::getInstance()->m_stockSetting[STOCK_TYPE_GAINIAN1].m_enable)
        {
            QVector<DayLineData>& dayLineDatas = StockDataManager::getInstance()->m_dataManagers[STOCK_TYPE_GAINIAN1]->m_dayLineDatas[STOCK_DATA_DAY];
            for (const auto& dayLineData : dayLineDatas)
            {
                // 只有一支股票，只需匹配日期
                if (stockData.m_beginTime == dayLineData.m_beginTime)
                {
                    gaiNian1ZhangDie = dayLineData.m_zhangFu;
                    break;
                }
            }
        }

        // 查找行业涨跌
        float hangYeZhangDie = 0.0f;
        if (SettingManager::getInstance()->m_stockSetting[STOCK_TYPE_HANGYE].m_enable)
        {
            QVector<DayLineData>& dayLineDatas = StockDataManager::getInstance()->m_dataManagers[STOCK_TYPE_HANGYE]->m_dayLineDatas[STOCK_DATA_DAY];
            for (const auto& dayLineData : dayLineDatas)
            {
                // 只有一支股票，只需匹配日期
                if (stockData.m_beginTime == dayLineData.m_beginTime)
                {
                    hangYeZhangDie = dayLineData.m_zhangFu;
                    break;
                }
            }
        }

        // 查找指数涨跌
        float zhiShuZhangDie = 0.0f;
        if (SettingManager::getInstance()->m_stockSetting[STOCK_TYPE_ZHISHU].m_enable)
        {
            QVector<DayLineData>& dayLineDatas = StockDataManager::getInstance()->m_dataManagers[STOCK_TYPE_ZHISHU]->m_dayLineDatas[STOCK_DATA_DAY];
            for (const auto& dayLineData : dayLineDatas)
            {
                // 只有一支股票，只需匹配日期
                if (stockData.m_beginTime == dayLineData.m_beginTime)
                {
                    zhiShuZhangDie = dayLineData.m_zhangFu;
                    break;
                }
            }
        }

        float score = geGuZhangDie - gaiNian2ZhangDie*SettingManager::getInstance()->m_stockSetting[STOCK_TYPE_GAINIAN2].m_kuoJianPercent
                - gaiNian1ZhangDie*SettingManager::getInstance()->m_stockSetting[STOCK_TYPE_GAINIAN1].m_kuoJianPercent
                - hangYeZhangDie*SettingManager::getInstance()->m_stockSetting[STOCK_TYPE_HANGYE].m_kuoJianPercent
                - zhiShuZhangDie*SettingManager::getInstance()->m_stockSetting[STOCK_TYPE_ZHISHU].m_kuoJianPercent;

        bool found = false;
        for (auto& stockScore : stockScores)
        {
            if (stockScore.m_industry == stockData.m_industryName
                    && stockScore.m_stockName == stockData.m_stockName)
            {
                found = true;
                stockScore.m_score += score;
                stockScore.m_totalDay += 1;
                break;
            }
        }

        if (!found)
        {
            StockScore stockScore;
            stockScore.m_industry = stockData.m_industryName;
            stockScore.m_stockName = stockData.m_stockName;
            stockScore.m_score = score;
            stockScore.m_totalDay = 1;
            stockScores.append(stockScore);
        }
    }

    return stockScores;
}

void StockDataWriter::writeSpaces(int count, QString& result)
{
    for (int i=0; i<count; i++)
    {
        result += " ";
    }
}

void StockDataWriter::writeStockXianContent(const StockData& stockData, QString& result)
{
    // 贵洲茅台 1岁大限	2001	2001	同(马)	梁(昌)(忌)马	机巨(禄)禄	空(存)(曲)(科)存	空	阳(权)权阴
    result += stockData.m_stockName + SPLIT_SPACE;

    // 查找该股票的限数据
    QVector<StockData> xianDatas;
    for (int i=0; i<ARRAY_SIZE(StockDataManager::getInstance()->m_dataManagers); i++)
    {
        DataManager* currentDataManager = StockDataManager::getInstance()->m_dataManagers[i];
        if (currentDataManager)
        {
            xianDatas = currentDataManager->getXianDatas(stockData.m_industryName, stockData.m_stockName);
            if (!xianDatas.empty())
            {
                break;
            }
        }
    }

    if (xianDatas.empty())
    {
        result += NEW_LINE;
        result += NEW_LINE;
        return;
    }

    // 输出1岁大限，第一个数据
    writeStockXianContent2(xianDatas[0], result);

    // 输出当前年份的大限
    qint64 now = QDateTime::currentSecsSinceEpoch();
    for (const StockData& xianData : xianDatas)
    {
        if (now >= xianData.m_beginTime && now <= xianData.m_endTime)
        {
            writeSpaces(stockData.m_stockName.length()*2, result);
            result += SPLIT_SPACE;
            writeStockXianContent2(xianData, result);
            break;
        }
    }

    result += NEW_LINE;
}

void StockDataWriter::writeStockXianContent2(const StockData& xianData, QString& result)
{
    result += xianData.m_lunarTime + SPLIT_SPACE;
    int beginYear = QDateTime::fromSecsSinceEpoch(xianData.m_beginTime).date().year();
    result += QString::number(beginYear) + SPLIT_SPACE;
    int endYear = QDateTime::fromSecsSinceEpoch(xianData.m_endTime).date().year();
    result += QString::number(endYear) + SPLIT_SPACE;
    for (int i=0; i<ARRAY_SIZE(xianData.m_data); i++)
    {
        result += xianData.m_data[i] + SPLIT_SPACE;
    }
    result += NEW_LINE;
}

void StockDataWriter::writeStockYearContent(const StockData& stockData, QString& result)
{
    // 从高到低获取日线数据（年）
    QVector<DayLineData> dayLineDatas;
    for (int i=STOCK_TYPE_ZHISHU; i>=STOCK_TYPE_GEGU; i--)
    {
        if (!SettingManager::getInstance()->m_stockSetting[i].m_enable)
        {
            continue;
        }

        QVector<DayLineData>& allYearDatas = StockDataManager::getInstance()->m_dataManagers[i]->m_dayLineDatas[STOCK_DATA_YEAR];
        QVector<DayLineData> datas;
        for (const auto& yearData : allYearDatas)
        {
            if (stockData.m_beginTime >= yearData.m_beginTime && stockData.m_beginTime <= yearData.m_endTime)
            {
                datas.append(yearData);
            }
        }

        if (datas.empty())
        {
            continue;
        }

        if (datas.size() == 1)
        {
            dayLineDatas.append(datas[0]);
            continue;
        }

        for (const auto& data : datas)
        {
            if (data.m_industryName == stockData.m_industryName && data.m_stockName == stockData.m_stockName)
            {
                dayLineDatas.append(data);
                break;
            }
        }
    }

    // 获取股票的年的数据
    StockData currentYearStockData;
    for (int i=STOCK_TYPE_GEGU; i<=STOCK_TYPE_ZHISHU; i++)
    {
        if (!SettingManager::getInstance()->m_stockSetting[i].m_enable)
        {
            continue;
        }

        QVector<StockData>& yearStockDatas = StockDataManager::getInstance()->m_dataManagers[i]->m_stockDatas[STOCK_DATA_YEAR];
        for (const auto& yearStockData : yearStockDatas)
        {
            if (yearStockData.m_industryName == stockData.m_industryName
                    && yearStockData.m_stockName == stockData.m_stockName
                    && stockData.m_beginTime >= yearStockData.m_beginTime
                    && stockData.m_endTime <= yearStockData.m_endTime)
            {
                currentYearStockData = yearStockData;
                break;
            }
        }

        break;
    }

    // 输出日线数据
    for (int i=0; i<dayLineDatas.size(); i++)
    {
        if (i!= 0 && i == dayLineDatas.size() - 1)
        {
            // 最后一个要换行
            result += NEW_LINE;
        }
        result += dayLineDatas[i].m_stockName + SPLIT_SPACE;
        result += currentYearStockData.m_lunarTime + SPLIT_SPACE;
        result += QString::number(dayLineDatas[i].m_zhangFu, 'f', 2) + "%" + SPLIT_SPACE;
        result += QString::fromWCharArray(L"共%1天").arg(dayLineDatas[i].m_totalDay) + SPLIT_SPACE;
        QString huanShou = QString::number(dayLineDatas[i].m_huanShou, 'f', 2);
        result += QString::fromWCharArray(L"换手%1").arg(huanShou) + "%" + SPLIT_SPACE;
    }    

    // 输出年6宫内容
    for (int j=0; j<ARRAY_SIZE(currentYearStockData.m_data); j++)
    {
        result += currentYearStockData.m_data[j] + SPLIT_SPACE;
    }

    result += NEW_LINE;
    result += NEW_LINE;
}

void StockDataWriter::writeStockMonthContent(const StockData& stockData, QString& result)
{
    // 从高到低获取日线数据（月）
    QVector<DayLineData> dayLineDatas;
    for (int i=STOCK_TYPE_ZHISHU; i>=STOCK_TYPE_GEGU; i--)
    {
        if (!SettingManager::getInstance()->m_stockSetting[i].m_enable)
        {
            continue;
        }

        QVector<DayLineData>& allMonthDatas = StockDataManager::getInstance()->m_dataManagers[i]->m_dayLineDatas[STOCK_DATA_MONTH];
        QVector<DayLineData> datas;
        for (const auto& monthData : allMonthDatas)
        {
            if (stockData.m_beginTime >= monthData.m_beginTime && stockData.m_beginTime <= monthData.m_endTime)
            {
                datas.append(monthData);
            }
        }

        if (datas.empty())
        {
            continue;
        }

        if (datas.size() == 1)
        {
            dayLineDatas.append(datas[0]);
            continue;
        }

        for (const auto& data : datas)
        {
            if (data.m_industryName == stockData.m_industryName && data.m_stockName == stockData.m_stockName)
            {
                dayLineDatas.append(data);
                break;
            }
        }
    }

    // 获取股票的月的数据
    StockData currentMonthStockData;
    for (int i=STOCK_TYPE_GEGU; i<=STOCK_TYPE_ZHISHU; i++)
    {
        if (!SettingManager::getInstance()->m_stockSetting[i].m_enable)
        {
            continue;
        }

        QVector<StockData>& monthStockDatas = StockDataManager::getInstance()->m_dataManagers[i]->m_stockDatas[STOCK_DATA_MONTH];
        for (const auto& monthStockData : monthStockDatas)
        {
            if (monthStockData.m_industryName == stockData.m_industryName
                    && monthStockData.m_stockName == stockData.m_stockName
                    && stockData.m_beginTime >= monthStockData.m_beginTime
                    && stockData.m_endTime <= monthStockData.m_endTime)
            {
                currentMonthStockData = monthStockData;
                break;
            }
        }

        break;
    }

    // 输出日线数据
    for (int i=0; i<dayLineDatas.size(); i++)
    {
        if (i != 0 && i == dayLineDatas.size() - 1)
        {
            // 最后一个要换行
            result += NEW_LINE;
        }
        result += dayLineDatas[i].m_stockName + SPLIT_SPACE;
        result += currentMonthStockData.m_lunarTime + SPLIT_SPACE;
        result += QString::number(dayLineDatas[i].m_zhangFu, 'f', 2) + "%" + SPLIT_SPACE;
        result += QString::fromWCharArray(L"共%1天").arg(dayLineDatas[i].m_totalDay) + SPLIT_SPACE;
        QString huanShou = QString::number(dayLineDatas[i].m_huanShou, 'f', 2);
        result += QString::fromWCharArray(L"换手%1").arg(huanShou) + "%" + SPLIT_SPACE;
    }

    // 输出6宫内容
    for (int j=0; j<ARRAY_SIZE(currentMonthStockData.m_data); j++)
    {
        result += currentMonthStockData.m_data[j] + SPLIT_SPACE;
    }

    result += NEW_LINE;
    result += NEW_LINE;
}

void StockDataWriter::writeStockDayContent(const StockData& stockData, QString& result)
{
    // 从高到低获取日线数据（日）
    QVector<DayLineData> dayLineDatas;
    for (int i=STOCK_TYPE_ZHISHU; i>=STOCK_TYPE_GEGU; i--)
    {
        if (!SettingManager::getInstance()->m_stockSetting[i].m_enable)
        {
            continue;
        }

        QVector<DayLineData>& allDayDatas = StockDataManager::getInstance()->m_dataManagers[i]->m_dayLineDatas[STOCK_DATA_DAY];
        QVector<DayLineData> datas;
        for (const auto& dayData : allDayDatas)
        {
            if (stockData.m_beginTime == dayData.m_beginTime)
            {
                datas.append(dayData);
            }
        }

        if (datas.empty())
        {
            continue;
        }

        if (datas.size() == 1)
        {
            dayLineDatas.append(datas[0]);
            continue;
        }

        for (const auto& data : datas)
        {
            if (data.m_industryName == stockData.m_industryName && data.m_stockName == stockData.m_stockName)
            {
                dayLineDatas.append(data);
                break;
            }
        }
    }

    QString thisDay = QDateTime::fromSecsSinceEpoch(stockData.m_beginTime).toString(QString::fromWCharArray(L"yyyy年MM月dd日"));

    // 输出日线数据
    for (int i=0; i<dayLineDatas.size(); i++)
    {
        if (i != 0 && i == dayLineDatas.size() - 1)
        {
            // 最后一个要换行
            result += NEW_LINE;
        }
        result += dayLineDatas[i].m_stockName + SPLIT_SPACE;
        result += thisDay + SPLIT_SPACE;
        result += QString::number(dayLineDatas[i].m_zhangFu, 'f', 2) + "%" + SPLIT_SPACE;
        QString huanShou = QString::number(dayLineDatas[i].m_huanShou, 'f', 2);
        result += QString::fromWCharArray(L"换手%1").arg(huanShou) + "%" + SPLIT_SPACE;
    }

    // 输出6宫内容
    for (int j=0; j<ARRAY_SIZE(stockData.m_data); j++)
    {
        result += stockData.m_data[j] + SPLIT_SPACE;
    }

    result += NEW_LINE;
    result += NEW_LINE;
}
