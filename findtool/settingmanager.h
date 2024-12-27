#pragma once

#include <QString>

class SettingManager
{
protected:
    SettingManager();

public:
    static SettingManager* getInstance();

    void save();    

private:
    void load();

public:
    int m_logLevel = 2;  // info

    int m_beforeYear = 0;

    int m_beforeMonth = 0;

    int m_beforeHour = 0;

    int m_beforeDay = 0;

    int m_afterYear = 0;

    int m_afterMonth = 0;

    int m_afterDay = 0;

    int m_afterHour = 0;
};
