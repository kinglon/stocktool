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
    std::wstring strConfFilePath = CImPath::GetConfPath() + L"findtool.json";
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

    m_logLevel = root["logLevel"].toInt();
    m_beforeYear = root["beforeYear"].toInt();
    m_beforeMonth = root["beforeMonth"].toInt();
    m_beforeDay = root["beforeDay"].toInt();
    m_beforeHour = root["beforeHour"].toInt();
    m_afterYear = root["afterYear"].toInt();
    m_afterMonth = root["afterMonth"].toInt();
    m_afterDay = root["afterDay"].toInt();
    m_afterHour = root["afterHour"].toInt();
    if (root.contains("stockRootPath"))
    {
        m_stockRootPath = root["stockRootPath"].toString();
    }
}

void SettingManager::save()
{
    QJsonObject root;
    root["logLevel"] = m_logLevel;
    root["beforeYear"] = m_beforeYear;
    root["beforeMonth"] = m_beforeMonth;
    root["beforeDay"] = m_beforeDay;
    root["beforeHour"] = m_beforeHour;
    root["afterYear"] = m_afterYear;
    root["afterMonth"] = m_afterMonth;
    root["afterDay"] = m_afterDay;
    root["afterHour"] = m_afterHour;
    root["stockRootPath"] = m_stockRootPath;

    QJsonDocument jsonDocument(root);
    QByteArray jsonData = jsonDocument.toJson(QJsonDocument::Indented);
    std::wstring strConfFilePath = CImPath::GetConfPath() + L"findtool.json";
    QFile file(QString::fromStdWString(strConfFilePath));
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        qCritical("failed to save setting");
        return;
    }
    file.write(jsonData);
    file.close();
}
