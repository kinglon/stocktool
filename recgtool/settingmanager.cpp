#include "settingmanager.h"
#include <QFile>
#include "../Utility/ImPath.h"
#include "../Utility/ImCharset.h"
#include "../Utility/LogMacro.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

SettingManager::SettingManager()
{
    load();
}

SettingManager* SettingManager::getInstance()
{
    static SettingManager* pInstance = new SettingManager();
	return pInstance;
}

void SettingManager::load()
{
    std::wstring strConfFilePath = CImPath::GetConfPath() + L"configs.json";    
    QFile file(QString::fromStdWString(strConfFilePath));
    if (!file.exists())
    {
        return;
    }

    if (!file.open(QIODevice::ReadOnly))
    {
        LOG_ERROR(L"failed to open the basic configure file : %s", strConfFilePath.c_str());
        return;
    }
    QByteArray jsonData = file.readAll();
    file.close();

    QJsonDocument jsonDocument = QJsonDocument::fromJson(jsonData);
    QJsonObject root = jsonDocument.object();

    m_nLogLevel = root["logLevel"].toInt();

    QJsonArray stockSettingJsonArray = root["stockSetting"].toArray();
    for (int i=0; i<STOCK_TYPE_MAX && i<stockSettingJsonArray.size(); i++)
    {
        QJsonObject stockSettingJsonObject = stockSettingJsonArray[i].toObject();
        m_stockSetting[i].m_stockPath = stockSettingJsonObject["path"].toString();
    }

    m_savedPath = root["savedPath"].toString();
}

void SettingManager::save()
{
    QJsonObject root;
    root["logLevel"] = m_nLogLevel;

    QJsonArray stockSettingJsonArray;
    for (int i=0; i<STOCK_TYPE_MAX; i++)
    {
        QJsonObject stockSettingJsonObject;
        stockSettingJsonObject["path"] = m_stockSetting[i].m_stockPath;
        stockSettingJsonArray.append(stockSettingJsonObject);
    }
    root["stockSetting"] = stockSettingJsonArray;
    root["savedPath"] = m_savedPath;

    QJsonDocument jsonDocument(root);
    QByteArray jsonData = jsonDocument.toJson(QJsonDocument::Indented);
    std::wstring strConfFilePath = CImPath::GetConfPath() + L"configs.json";
    QFile file(QString::fromStdWString(strConfFilePath));
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        qCritical("failed to save setting");
        return;
    }
    file.write(jsonData);
    file.close();
}
