#include "stockdatamanager.h"

StockDataManager::StockDataManager()
{

}

StockDataManager* StockDataManager::getInstance()
{
    static StockDataManager* instance = new StockDataManager();
    return instance;
}

bool StockDataManager::saveResult(const QString& savePath)
{
    // todo by yejinlong
    return true;
}
