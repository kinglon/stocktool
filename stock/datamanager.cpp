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
    for (int i=0; i<MAX_STOCK_DATA_COUNT; i++)
    {
        std::sort(m_stockDatas[i].begin(), m_stockDatas[i].end(), [](const StockData& a, const StockData& b) {
            return a.m_beginTime < b.m_beginTime;
        });
    }
}
