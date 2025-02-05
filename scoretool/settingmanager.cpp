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
    for (int i=0; i<SCORE_COUNT; i++)
    {
        m_scores[i] = 0;
    }

    load();
    loadHoliday();
}

SettingManager* SettingManager::getInstance()
{
    static SettingManager* pInstance = new SettingManager();
	return pInstance;
}

void SettingManager::load()
{
    std::wstring strConfFilePath = CImPath::GetConfPath() + L"score_config.json";
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

    if (root.contains("score"))
    {
        QJsonArray scores = root["score"].toArray();
        for (int i=0; i<scores.size(); i++)
        {
            m_scores[i] = scores[i].toInt();
        }
    }
}

void SettingManager::save()
{
    QJsonObject root;
    root["log_level"] = m_nLogLevel;

    QJsonArray scores;
    for (int i=0; i<SCORE_COUNT; i++)
    {
        scores.append(m_scores[i]);
    }
    root["score"] = scores;

    QJsonDocument jsonDocument(root);
    QByteArray jsonData = jsonDocument.toJson(QJsonDocument::Indented);
    std::wstring strConfFilePath = CImPath::GetConfPath() + L"score_config.json";
    QFile file(QString::fromStdWString(strConfFilePath));
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        qCritical("failed to save setting");
        return;
    }
    file.write(jsonData);
    file.close();
}

void SettingManager::loadHoliday()
{
    std::wstring settingFilePath = CImPath::GetConfPath() + L"holiday.txt";
    QFile file(QString::fromStdWString(settingFilePath));
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qCritical("failed to open holiday file");
        return;
    }

    QTextStream in(&file);
    in.setCodec("UTF-8");
    while (!in.atEnd())
    {
        QStringList fields = in.readLine().split(" ");
        if (fields.size() < 3)
        {
            continue;
        }

        QDate begin = QDate::fromString(fields[1], "yyyy-M-d");
        QDate end = QDate::fromString(fields[2], "yyyy-M-d");
        if (begin > end)
        {
            continue;
        }

        DateRange dateRange;
        dateRange.m_begin = begin.startOfDay().toSecsSinceEpoch();
        dateRange.m_end = end.startOfDay().toSecsSinceEpoch();
        m_holidays.append(dateRange);
    }
    file.close();
}
