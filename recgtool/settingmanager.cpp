﻿#include "settingmanager.h"
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
        m_stockSetting[i].m_enableZhangDie = stockSettingJsonObject["enableZhangDie"].toBool();
        m_stockSetting[i].m_zhangDieStart = stockSettingJsonObject["zhangDieStart"].toDouble();
        m_stockSetting[i].m_zhangDieEnd = stockSettingJsonObject["zhangDieEnd"].toDouble();
        m_stockSetting[i].m_kuoJianPercent = stockSettingJsonObject["kuoJianPercent"].toDouble();
    }

    m_savedPath = root["savedPath"].toString();
    m_cacheData = root["cacheData"].toBool();
    m_recgDateStart = root["recgDateStart"].toInt();
    m_recgDateEnd = root["recgDateEnd"].toInt();
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
        stockSettingJsonObject["enableZhangDie"] = m_stockSetting[i].m_enableZhangDie;
        stockSettingJsonObject["zhangDieStart"] = m_stockSetting[i].m_zhangDieStart;
        stockSettingJsonObject["zhangDieEnd"] = m_stockSetting[i].m_zhangDieEnd;
        stockSettingJsonObject["kuoJianPercent"] = m_stockSetting[i].m_kuoJianPercent;
        stockSettingJsonArray.append(stockSettingJsonObject);
    }
    root["stockSetting"] = stockSettingJsonArray;
    root["savedPath"] = m_savedPath;
    root["cacheData"] = m_cacheData;
    root["recgDateStart"] = m_recgDateStart;
    root["recgDateEnd"] = m_recgDateEnd;

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
