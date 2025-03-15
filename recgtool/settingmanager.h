#pragma once

#include <QString>
#include <QVector>
#include "../stock/stockdatautil.h"

// 股票类别
#define STOCK_TYPE_GEGU  0
#define STOCK_TYPE_GAINIAN2  1
#define STOCK_TYPE_GAINIAN1  2
#define STOCK_TYPE_HANGYE  3
#define STOCK_TYPE_ZHISHU  4
#define STOCK_TYPE_MAX  STOCK_TYPE_ZHISHU+1

// 查找方法
#define SEARCH_TYPE_MIN        1
#define SEARCH_TYPE_JINGQUE    1
#define SEARCH_TYPE_KEY_WORD    2
#define SEARCH_TYPE_ALGORITHM    3
#define SEARCH_TYPE_MAX    3

// 比对内容来源
#define COMPARE_CONTENT_ZHIDING_WORD    1  //指定字
#define COMPARE_CONTENT_ALL_WORD    2  //全部字
#define COMPARE_CONTENT_ZHIDING_YEAR    3  //指定年的股票内容获取
#define COMPARE_CONTENT_ZHIDING_MONTH    4  //指定月的股票内容获取
#define COMPARE_CONTENT_ZHIDING_DAY    5  //指定日的股票内容获取

class StockSetting
{
public:
    // 是否需要加入筛选
    bool m_enable = false;

    // 股票路径
    QString m_stockPath;

    // 是否需要根据涨跌幅筛选
    bool m_enableZhangDie = false;

    // 涨跌范围百分比，（不含%，10%取10）
    float m_zhangDieStart = 0.0f;
    float m_zhangDieEnd = 0.0f;

    // 值算法使用，扣减百分比，已经除以100了，10%取0.1
    float m_kuoJianPercent = 0.0f;
};

class SettingManager
{
protected:
    SettingManager();

public:
    static SettingManager* getInstance();

    void save();

    // 开启debug级别日志
    bool enableDebugLog() { return m_nLogLevel==1; }

private:
    void load();

public:
    int m_nLogLevel = 2;  // info

    StockSetting m_stockSetting[STOCK_TYPE_MAX];

    // 查找方法
    int m_searchMethod = SEARCH_TYPE_ALGORITHM;

    // 标志是否全宫相关
    bool m_allMatch = true;

    // 标志是否按降序排
    bool m_sortDesc = true;

    // 比对内容来源
    int m_compareContentFrom = COMPARE_CONTENT_ZHIDING_DAY;

    // 比对指定日期
    qint64 m_compareDate = 0;

    // 指定字的筛选条件
    FilterCondition m_zhiDingWordFilterCondition;

    // 全部字的筛选条件
    FilterCondition m_allWordFilterCondition;

    // 识别内容时间范围, utc时间戳，单位秒
    qint64 m_recgDateStart = 0;
    qint64 m_recgDateEnd = 0;

    // 保存路径
    QString m_savedPath;
};
