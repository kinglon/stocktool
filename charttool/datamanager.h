#pragma once

#include <QString>
#include <QVector>
#include <QColor>

#define CHART_DATA_TYPE_MONTH   1
#define CHART_DATA_TYPE_DAY     2

// 颜色
#define RGB_RED       (QColor(255, 0, 0).rgb())
#define RGB_GREEN     (QColor(0, 255, 0).rgb())
#define RGB_ORANGE    (QColor(255, 165, 0).rgb())

class ChartData
{
public:
    // 类型
    int m_type = CHART_DATA_TYPE_DAY;

    // 日期的时间戳，单位秒
    qint64 m_date = 0;

    // 个数
    int m_count = 0;

    // 颜色
    QRgb m_color;

public:
    void switchNextColor()
    {
        if (m_color == RGB_RED)
        {
            m_color = RGB_GREEN;
        }
        else if (m_color == RGB_GREEN)
        {
            m_color = RGB_ORANGE;
        }
        else if (m_color == RGB_ORANGE)
        {
            m_color = RGB_RED;
        }
    }
};

class AvgLine
{
public:
    // 表格数据开始索引
    int m_begin = 0;

    // 表格数据结束索引（含）
    int m_end = 0;

    // 平均格式
    int m_count = 1;
};

class ColorData
{
public:
    // 类型
    int m_type = CHART_DATA_TYPE_DAY;

    // 日期的时间戳，单位秒
    qint64 m_date = 0;

    // 标志是否一宫含
    bool oneInclude = false;

    // 标志是否二宫含
    bool twoInclude = false;
};

class HourData
{
public:
    // 日期的时间戳，单位秒
    qint64 m_date = 0;

    // 巳个数
    int m_siCount = 0;

    // 午个数
    int m_wuCount = 0;

    // 未个数
    int m_weiCount = 0;
};

class DataManager
{
protected:
    DataManager();

public:
    static DataManager* getInstance();

public:
    // 主能量数据
    QVector<ChartData> m_chartDatas;

    QVector<AvgLine> m_avgLines;

    // 辅助能量1数据
    QVector<ChartData> m_assist1Datas;

    // 辅助能量2数据
    QVector<ChartData> m_assist2Datas;

    // 主色块数据
    QVector<ColorData> m_colorDatas;

    // 辅1色块数据
    QVector<ColorData> m_assist1ColorDatas;

    // 辅2色块数据
    QVector<ColorData> m_assist2ColorDatas;

    // 时数据
    QVector<HourData> m_hourDatas;
};
