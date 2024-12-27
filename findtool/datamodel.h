#ifndef DATAMODEL_H
#define DATAMODEL_H

#include <QString>

class Stock
{
public:
    // 股票名字
    QString m_stockName;

    // 股票代码
    QString m_stockId;

    // 股票数据完整目录
    QString m_stockPath;
};

#define DATA_FIELD_LENGTH   4

// 用来存储一条股票信息
class StockData
{
public:
    // 行业
    QString m_industryName;

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
    QString m_data[DATA_FIELD_LENGTH];

public:
    // 把时中文变成数字，方便排序
    int hourToInt() const
    {
        if (m_hour == QString::fromWCharArray(L"巳"))
        {
            return 1;
        }
        else if (m_hour == QString::fromWCharArray(L"午"))
        {
            return 2;
        }
        else if (m_hour == QString::fromWCharArray(L"未"))
        {
            return 3;
        }
        else
        {
            return 100;
        }
    }
};

#endif // DATAMODEL_H
