#include "settingmanager.h"
#include <QFile>
#include "Utility/ImPath.h"
#include "Utility/ImCharset.h"
#include "Utility/LogMacro.h"
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

    if (root.contains("log_level"))
    {
        m_nLogLevel = root["log_level"].toInt();
    }

    if (root.contains("filter_condition"))
    {
        QJsonArray filterConditions = root["filter_condition"].toArray();
        for (int i=0; i<filterConditions.size(); i++)
        {
            QJsonObject filterConditionObject = filterConditions[i].toObject();
            m_filterCondition[i].m_oneExclude = filterConditionObject["one_exclude"].toString();
            m_filterCondition[i].m_oneInclude = filterConditionObject["one_include"].toString();
            m_filterCondition[i].m_twoExclude = filterConditionObject["two_exclude"].toString();
            m_filterCondition[i].m_twoInclude = filterConditionObject["two_include"].toString();
        }
    }
}

void SettingManager::save()
{
    QJsonObject root;
    root["log_level"] = m_nLogLevel;

    QJsonArray filterConditions;
    for (int i=0; i<MAX_FILTER_CONDTION_COUNT; i++)
    {
        QJsonObject filterConditionObject;
        filterConditionObject["one_exclude"] = m_filterCondition[i].m_oneExclude;
        filterConditionObject["one_include"] = m_filterCondition[i].m_oneInclude;
        filterConditionObject["two_exclude"] = m_filterCondition[i].m_twoExclude;
        filterConditionObject["two_include"] = m_filterCondition[i].m_twoInclude;
        filterConditions.append(filterConditionObject);
    }
    root["filter_condition"] = filterConditions;

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
