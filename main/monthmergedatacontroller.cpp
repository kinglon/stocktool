#include "monthmergedatacontroller.h"
#include "Utility/ImPath.h"
#include <QFile>
#include <QTextStream>

#define PREFIX_SPACE_COUNT 18

MonthDataMerger::MonthDataMerger(QObject *parent)
    : DataMerger{parent}
{

}

QString MonthDataMerger::getCompareResult()
{
    if (m_stockName2MonthList == nullptr || m_dayLineDatas.empty())
    {
        return "";
    }

    QString stockName = m_dayLineDatas[0].m_dayStockData.m_stockName;
    if (!m_stockName2MonthList->contains(stockName))
    {
        return "";
    }

    QVector<QString>& months = (*m_stockName2MonthList)[stockName];
    if (months.empty())
    {
        return "";
    }

    QString year;
    QString result;
    QString seperator = "    ";
    for (int i=0; i<months.size(); i++)
    {
        // 获取年份
        int yearIndex = months[i].indexOf(QString::fromWCharArray(L"年"));
        if (yearIndex == -1)
        {
            continue;
        }
        QString newYear = months[i].left(yearIndex);

        // 如果是一个新的年份，输出年的数据
        if (newYear != year)
        {
            year = newYear;
            StockData yearStockData;
            for (const auto& stockData : m_yearStockData)
            {
                if (stockData.m_lunarTime == newYear)
                {
                    yearStockData = stockData;
                    break;
                }
            }

            if (!yearStockData.m_stockName.isEmpty())
            {
                result += yearStockData.m_stockName;
                result += seperator + yearStockData.m_lunarTime;
                for (int i=0; i<DATA_FIELD_LENGTH; i++)
                {
                    result += seperator + yearStockData.m_data[i];
                }
                result += "\r\n";

                // 输出年对比匹配的数据
                QVector<DayLineData> dayLineDatas;
                filterData(yearStockData, dayLineDatas);
                if (dayLineDatas.empty())
                {
                    appendSpaceChar(result, PREFIX_SPACE_COUNT);
                    result += QString::fromWCharArray(L"无");
                    result += "\r\n\r\n";
                }
                else
                {
                    for (const auto& dayLineData : dayLineDatas)
                    {
                        appendSpaceChar(result, PREFIX_SPACE_COUNT);
                        result += QDateTime::fromSecsSinceEpoch(dayLineData.m_dayStockData.m_beginTime).toString("yyyy/M/d");
                        result += seperator + dayLineData.m_zhangfu;
                        for (int i=0; i<DATA_FIELD_LENGTH; i++)
                        {
                            result += seperator + dayLineData.m_dayStockData.m_data[i];
                        }

                        StockData monthStockData = getMonthDataByDayData(dayLineData.m_dayStockData);
                        if (!monthStockData.m_stockName.isEmpty())
                        {
                            result += seperator + monthStockData.m_lunarTime;
                            for (int i=0; i<DATA_FIELD_LENGTH; i++)
                            {
                                result += seperator + monthStockData.m_data[i];
                            }
                        }

                        result += "\r\n\r\n";
                    }
                }
                result += "\r\n\r\n\r\n";
            }
        }

        // 输出本月信息
        StockData monthStockData;
        for (const auto& stockData : m_monthStockData)
        {
            if (stockData.m_lunarTime == months[i])
            {
                monthStockData = stockData;
                break;
            }
        }

        if (!monthStockData.m_stockName.isEmpty())
        {
            appendSpaceChar(result, PREFIX_SPACE_COUNT);
            result += monthStockData.m_lunarTime;
            for (int i=0; i<DATA_FIELD_LENGTH; i++)
            {
                result += seperator + monthStockData.m_data[i];
            }
            result += "\r\n";

            // 输出月对比匹配的数据
            QVector<DayLineData> dayLineDatas;
            filterData(monthStockData, dayLineDatas);
            if (dayLineDatas.empty())
            {
                appendSpaceChar(result, PREFIX_SPACE_COUNT);
                result += seperator;
                result += QString::fromWCharArray(L"无");
                result += "\r\n\r\n";
            }
            else
            {
                for (const auto& dayLineData : dayLineDatas)
                {
                    appendSpaceChar(result, PREFIX_SPACE_COUNT);
                    result += seperator;
                    result += QDateTime::fromSecsSinceEpoch(dayLineData.m_dayStockData.m_beginTime).toString("yyyy/M/d");
                    result += seperator + dayLineData.m_zhangfu;
                    for (int i=0; i<DATA_FIELD_LENGTH; i++)
                    {
                        result += seperator + dayLineData.m_dayStockData.m_data[i];
                    }

                    StockData monthStockData = getMonthDataByDayData(dayLineData.m_dayStockData);
                    if (!monthStockData.m_stockName.isEmpty())
                    {
                        result += seperator + monthStockData.m_lunarTime;
                        for (int i=0; i<DATA_FIELD_LENGTH; i++)
                        {
                            result += seperator + monthStockData.m_data[i];
                        }
                    }

                    result += "\r\n\r\n";
                }
            }
        }
    }

    return result;
}

void MonthDataMerger::filterData(const StockData& stockData, QVector<DayLineData>& dayLineDatas)
{
    for (int i=0; i<m_dayLineDatas.size(); i++)
    {
        bool ok = false;
        if (m_compare2Part)
        {
            if (stockData.m_data[0] == m_dayLineDatas[i].m_dayStockData.m_data[0]
                    && stockData.m_data[1] == m_dayLineDatas[i].m_dayStockData.m_data[1])
            {
                ok = true;
            }
        }
        else
        {
            ok = true;
            for (int j=0; j<DATA_FIELD_LENGTH;j++)
            {
                if (stockData.m_data[j] != m_dayLineDatas[i].m_dayStockData.m_data[j])
                {
                    ok = false;
                    break;
                }
            }
        }

        if (ok)
        {
            dayLineDatas.append(m_dayLineDatas[i]);
        }
    }
}

MonthMergeDataController::MonthMergeDataController(QObject *parent)
    : MergeDataController{parent}
{

}

void MonthMergeDataController::loadResultData()
{
    QString resultFilePath = QString::fromStdWString(CImPath::GetDataPath()) + QString::fromWCharArray(L"结果.txt");
    QFile resultFile(resultFilePath);
    if (resultFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QTextStream in(&resultFile);
        in.setCodec("UTF-8");
        while (!in.atEnd())
        {
            QString line = in.readLine();
            if (line.isEmpty())
            {
                continue;
            }

            QStringList fields = line.split(' ');
            if (fields.length() < 2)
            {
                continue;
            }

            QString month = fields[0];
            QDateTime dateTime = QDateTime::fromString(month, QString::fromWCharArray(L"yyyy年M月"));
            if (!dateTime.isValid())
            {
                break;
            }

            for (int i=1; i<fields.size(); i++)
            {
                if (!fields[i].isEmpty())
                {
                    m_stockName2MonthList[fields[i]].append(month);
                }
            }
        }
        resultFile.close();
    }
}

DataMerger* MonthMergeDataController::getDataMerger()
{
    if (!m_hasLoaded)
    {
        loadResultData();
        m_hasLoaded = true;
    }

    MonthDataMerger* dataMerger = new MonthDataMerger();
    dataMerger->m_stockName2MonthList = &m_stockName2MonthList;
    return dataMerger;
}
