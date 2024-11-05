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
    load();
}

DataManager* DataManager::getInstance()
{
    static DataManager* pInstance = new DataManager();
	return pInstance;
}

void DataManager::load()
{
    std::wstring strConfFilePath = CImPath::GetConfPath() + L"calendar_data.json";
    QFile file(QString::fromStdWString(strConfFilePath));
    if (!file.exists())
    {
        return;
    }

    if (!file.open(QIODevice::ReadOnly))
    {
        LOG_ERROR(L"failed to open the calendar data file : %s", strConfFilePath.c_str());
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

    if (root.contains("data"))
    {
        QJsonArray calendarDatas = root["data"].toArray();
        for (int i=0; i<calendarDatas.size(); i++)
        {
            QJsonObject calendarDataObject = calendarDatas[i].toObject();
            CalendarData calendarData;
            calendarData.m_date = (qint64)(calendarDataObject["date"].toDouble());
            calendarData.m_value = calendarDataObject["value"].toInt();
            m_calendarDatas.append(calendarData);
        }
    }
}

void DataManager::save()
{
    QJsonObject root;
    root["log_level"] = m_nLogLevel;

    QJsonArray calendarDatas;
    for (int i=0; i<m_calendarDatas.size(); i++)
    {
        QJsonObject calendarDataObject;
        calendarDataObject["date"] = m_calendarDatas[i].m_date;
        calendarDataObject["value"] = m_calendarDatas[i].m_value;
        calendarDatas.append(calendarDataObject);
    }
    root["data"] = calendarDatas;

    QJsonDocument jsonDocument(root);
    QByteArray jsonData = jsonDocument.toJson(QJsonDocument::Indented);
    std::wstring strConfFilePath = CImPath::GetConfPath() + L"calendar_data.json";
    QFile file(QString::fromStdWString(strConfFilePath));
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        qCritical("failed to save setting");
        return;
    }
    file.write(jsonData);
    file.close();
}

void DataManager::addValue(qint64 date, int value)
{
    for (auto& data : m_calendarDatas)
    {
        if (data.m_date == date)
        {
            data.m_value = value;
            return;
        }
    }

    CalendarData data;
    data.m_date = date;
    data.m_value = value;
    m_calendarDatas.append(data);
    save();
}

int DataManager::getValue(qint64 date)
{
    for (auto& data : m_calendarDatas)
    {
        if (data.m_date == date)
        {
            return data.m_value;
        }
    }

    return 0;
}
