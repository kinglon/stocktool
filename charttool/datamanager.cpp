#include "datamanager.h"
#include <QFile>
#include "Utility/ImPath.h"
#include "Utility/ImCharset.h"
#include "Utility/LogMacro.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

DataManager::DataManager()
{

}

DataManager* DataManager::getInstance()
{
    static DataManager* pInstance = new DataManager();
	return pInstance;
}
