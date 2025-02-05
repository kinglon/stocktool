#pragma once

#include <QString>
#include <QVector>

#define SCORE_COUNT 12

class DateRange
{
public:
    // utc 时间戳
    qint64 m_begin = 0;
    qint64 m_end = 0; // 含
};

class SettingManager
{
protected:
    SettingManager();

public:
    static SettingManager* getInstance();

    void save();    

private:
    void load();

    void loadHoliday();

public:
    int m_nLogLevel = 2;  // info

    // 一宫禄权科忌存羊，二宫禄权科忌存羊
    int m_scores[SCORE_COUNT];

    // 假期
    QVector<DateRange> m_holidays;
};
