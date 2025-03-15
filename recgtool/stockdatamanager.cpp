#include "stockdatamanager.h"

StockDataManager::StockDataManager()
{
    for (int i=0; i<sizeof(m_dataManagers)/sizeof(m_dataManagers[0]); i++)
    {
        m_dataManagers[i] = DataManager::createInstance();
    }
}

StockDataManager* StockDataManager::getInstance()
{
    static StockDataManager* instance = new StockDataManager();
    return instance;
}

bool StockDataManager::hasData()
{
    for (int i=0; i<sizeof(m_dataManagers)/sizeof(m_dataManagers[0]); i++)
    {
        if (m_dataManagers[i]->hasData())
        {
            return true;
        }
    }

    return false;
}

void StockDataManager::clear()
{
    m_result = "";
    for (int i=0; i<sizeof(m_dataManagers)/sizeof(m_dataManagers[0]); i++)
    {
        m_dataManagers[i]->clearData();
    }
}
