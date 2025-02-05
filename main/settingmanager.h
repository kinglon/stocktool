#pragma once

#include <QString>
#include <QVector>
#include "datamodel.h"
#include "../stock/stockdatautil.h"

// 过滤条件索引
#define YEAR_FILTER_CONDTION        0
#define MONTH_FILTER_CONDTION       1
#define DAY_FILTER_CONDTION         2
#define SECOND_DAY_FILTER_CONDTION  3
#define YI_HOUR_FILTER_CONDTION     4
#define WU_HOUR_FILTER_CONDTION     5
#define WEI_HOUR_FILTER_CONDTION    6
#define MAX_FILTER_CONDTION_COUNT   7   // 过滤条件个数

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
    int m_nLogLevel = 2;  // info

    FilterCondition m_filterCondition[MAX_FILTER_CONDTION_COUNT];

    // 无损筛选条件
    FilterConditionV2 m_filterConditionV2;
};
