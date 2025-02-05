#ifndef STOCKDATAUTIL_H
#define STOCKDATAUTIL_H

#include <QString>

// 股票数据索引
#define STOCK_DATA_YEAR         0
#define STOCK_DATA_MONTH        1
#define STOCK_DATA_DAY          2
#define STOCK_DATA_HOUR         3
#define STOCK_DATA_HOUR_YI      3
#define STOCK_DATA_HOUR_WU      4
#define STOCK_DATA_HOUR_WEI     5
#define MAX_STOCK_DATA_COUNT    6

#define DATA_FIELD_LENGTH   6

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
    bool isEnable() const
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

class FilterItem
{
public:
    // 含的字
    QString m_include;

    // 不含的字
    QString m_exclude;
};

// 无损筛选，只查关键字
class FilterConditionV2
{
public:
    FilterItem m_filterItems[DATA_FIELD_LENGTH];

public:
    bool isEnable() const
    {
        for (int i=0; i<DATA_FIELD_LENGTH; i++)
        {
            if (!m_filterItems[i].m_include.isEmpty() || !m_filterItems[i].m_exclude.isEmpty())
            {
                return true;
            }
        }

        return false;
    }
};

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

    // 数据，6个字段，又称一二三四五六宫
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

class StockDataUtil
{
public:    
    // 检查是否满足条件
    // matchAll True 全宫相关匹配，False 一宫不与四六宫匹配，二宫不与三五宫匹配
    static bool checkIfStockDataOk(StockData stockData, const FilterCondition& filterCondition, bool matchAll);

    // 无损筛查，检查是否含设置的关键词
    static bool checkIfStockDataOkV2(StockData stockData, const FilterConditionV2& filterCondition);

    // 检查data是否有存字
    static bool hasCunWord(bool checkOneGong, QString gongDatas[DATA_FIELD_LENGTH], const QVector<int>& matchIndex);

    // 检查data是否有禄字
    static bool hasLuWord(bool checkOneGong, QString gongDatas[DATA_FIELD_LENGTH], const QVector<int>& matchIndex);

    // 检查data是否有忌字
    static bool hasJiWord(bool checkOneGong, QString gongDatas[DATA_FIELD_LENGTH], const QVector<int>& matchIndex);

    // 检查data是否有科字
    static bool hasKeWord(bool checkOneGong, QString gongDatas[DATA_FIELD_LENGTH], const QVector<int>& matchIndex);

    // 检查data是否有权字
    static bool hasQuanWord(bool checkOneGong, QString gongDatas[DATA_FIELD_LENGTH], const QVector<int>& matchIndex);

    // 检查是否有关键字，word只有一个字
    static bool hasWord(const QString& word, bool checkOneGong, QString gongDatas[DATA_FIELD_LENGTH], const QVector<int>& matchIndex);

    // 检查matchIndex指定的宫是否有不带括号的字
    static bool haveWordWithoutKuohao(const QString& word, QString data[DATA_FIELD_LENGTH], const QVector<int>& matchIndex);

    // 检查data是否有不带括号的字
    static bool haveWordWithoutKuohao(const QString& word, const QString& data);

    // 解析股票一行数据
    // dataType STOCK_DATA_*
    static bool parseOneLine(const QString& industryName, const QString& stockName, int dataType, const QString& line, StockData& stockData);
};

#endif // STOCKDATAUTIL_H
