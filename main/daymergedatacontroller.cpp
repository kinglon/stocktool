#include "daymergedatacontroller.h"

DayDataMerger::DayDataMerger(QObject *parent)
    : DataMerger{parent}
{

}

QString DayDataMerger::getCompareResult()
{
    int begin = 0;
    for (; begin<m_dayStockData.size(); begin++)
    {
        if (m_dayStockData[begin].m_beginTime >= m_beginDate)
        {
            break;
        }
    }

    QString result;
    for (; begin<=m_dayStockData.size(); begin++)
    {
        if (m_dayStockData[begin].m_beginTime >= m_endDate)
        {
            break;
        }

        QVector<DayLineData> dayLineDatas;
        filterData(m_dayStockData[begin], dayLineDatas);
        appendData(m_dayStockData[begin], dayLineDatas, result);
    }

    return result;
}

void DayDataMerger::filterData(const StockData& dayStockData, QVector<DayLineData>& dayLineDatas)
{
    for (int i=0; i<m_dayLineDatas.size(); i++)
    {
        if (m_dayLineDatas[i].m_dayStockData.m_beginTime >= dayStockData.m_beginTime)
        {
            break;
        }

        bool ok = false;
        if (m_compare2Part)
        {
            if (dayStockData.m_data[0] == m_dayLineDatas[i].m_dayStockData.m_data[0]
                    && dayStockData.m_data[1] == m_dayLineDatas[i].m_dayStockData.m_data[1])
            {
                ok = true;
            }
        }
        else
        {
            ok = true;
            for (int j=0; j<DATA_FIELD_LENGTH;j++)
            {
                if (dayStockData.m_data[j] != m_dayLineDatas[i].m_dayStockData.m_data[j])
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

void DayDataMerger::appendData(const StockData& dayStockData, const QVector<DayLineData>& dayLineDatas, QString& result)
{
    QString seperator = "    ";

    // 输出比对股票日月信息
    result += dayStockData.m_stockName;
    result += seperator + QDateTime::fromSecsSinceEpoch(dayStockData.m_beginTime).toString("yyyy/M/d");
    for (int i=0; i<DATA_FIELD_LENGTH; i++)
    {
        result += seperator + dayStockData.m_data[i];
    }

    StockData monthStockData = getMonthDataByDayData(dayStockData);
    if (!monthStockData.m_stockName.isEmpty())
    {
        result += seperator + monthStockData.m_lunarTime;
        for (int i=0; i<DATA_FIELD_LENGTH; i++)
        {
            result += seperator + monthStockData.m_data[i];
        }
    }

    result += "\r\n\r\n";

    if (dayLineDatas.isEmpty())
    {
        result += QString::fromWCharArray(L"无");
        result += "\r\n";
    }
    else
    {
        // 一行行输出匹配的日月信息
        for (auto& dayLineData : dayLineDatas)
        {
            result += dayLineData.m_dayStockData.m_stockName;
            result += seperator + QDateTime::fromSecsSinceEpoch(dayLineData.m_dayStockData.m_beginTime).toString("yyyy/M/d");
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

            result += "\r\n";
        }
    }
    result += "\r\n";
}

DayMergeDataController::DayMergeDataController(QObject *parent)
    : MergeDataController{parent}
{

}

DataMerger* DayMergeDataController::getDataMerger()
{
    DayDataMerger* dataMerger = new DayDataMerger();
    dataMerger->m_beginDate = m_beginDate;
    dataMerger->m_endDate = m_endDate;
    return dataMerger;
}
