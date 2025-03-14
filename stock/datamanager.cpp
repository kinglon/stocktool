#include "datamanager.h"

DataManager::DataManager()
{

}

DataManager* DataManager::getInstance()
{
    static DataManager* instance = new DataManager();
    return instance;
}

bool DataManager::hasData()
{
    for (int i=0; i<MAX_STOCK_DATA_COUNT; i++)
    {
        if (m_stockDatas[i].size() > 0)
        {
            return true;
        }
    }
    return false;
}

void DataManager::clearData()
{
    for (int i=0; i<MAX_STOCK_DATA_COUNT; i++)
    {
        m_stockDatas[i].clear();
    }

    for (int i=0; i<sizeof(m_dayLineDatas)/sizeof(m_dayLineDatas[0]); i++)
    {
        m_dayLineDatas[i].clear();
    }
}

int DataManager::totalCount()
{
    int totalCount = 0;
    for (int i=0; i<MAX_STOCK_DATA_COUNT; i++)
    {
        totalCount += m_stockDatas[i].size();
    }
    return totalCount;
}

void DataManager::sort()
{
    for (int i=0; i<sizeof(m_stockDatas)/sizeof(m_stockDatas[0]); i++)
    {
        std::sort(m_stockDatas[i].begin(), m_stockDatas[i].end(), [](const StockData& a, const StockData& b) {
            return a.m_beginTime < b.m_beginTime;
        });
    }

    for (int i=0; i<sizeof(m_dayLineDatas)/sizeof(m_dayLineDatas[0]); i++)
    {
        std::sort(m_dayLineDatas[i].begin(), m_dayLineDatas[i].end(), [](const DayLineData& a, const DayLineData& b) {
            return a.m_beginTime < b.m_beginTime;
        });
    }
}

QVector<StockData> DataManager::getXianDatas(const QString& industry, const QString& stockName)
{
    QVector<StockData> datas;
    for (const auto& stockData : m_stockDatas[STOCK_DATA_XIAN])
    {
        if (stockData.m_industryName == industry && stockData.m_stockName == stockName)
        {
            datas.append(stockData);
        }
    }

    return datas;
}
