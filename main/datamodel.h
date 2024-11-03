#ifndef DATAMODEL_H
#define DATAMODEL_H

#include <QString>

// 筛选条件
class FilterCondition
{
public:
    // 一宫含的字
    QString m_oneInclude;

    // 一宫不含的字
    QString m_oneExclude;

    // 二宫含的字
    QString m_twoInclude;

    // 二宫不含的字
    QString m_twoExclude;

public:
    bool isEnable()
    {
        if (m_oneInclude.isEmpty() && m_oneExclude.isEmpty()
                && m_twoInclude.isEmpty() && m_twoExclude.isEmpty())
        {
            return false;
        }
        else
        {
            return true;
        }
    }
};

// 用来存储一条股票信息
class StockData
{
public:
    // 股票名称
    QString m_stockName;

    // 农历时间，年和月的数据才有
    QString m_lunarTime;

    // 公历时间的时间戳
    qint64 m_beginTime = 0;
    qint64 m_endTime = 0;

    // 时, 已午未
    QString m_hour;

    // 数据，4个字段，又称一二三四宫
    QString m_data[4];
};

#endif // DATAMODEL_H
