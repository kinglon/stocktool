#pragma once

#include <QString>
#include <QVector>

class CalendarData
{
public:
    // 日期的时间戳
    qint64 m_date = 0;

    // 数值
    int m_value = 0;
};

class DataManager
{
protected:
    DataManager();

public:
    static DataManager* getInstance();

    void save();

    void addValue(qint64 date, int value);

    // 没找到，返回0
    int getValue(qint64 date);

private:
    void load();

public:
    int m_nLogLevel = 2;  // info

    QVector<CalendarData> m_calendarDatas;
};
