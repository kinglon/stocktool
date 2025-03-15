#ifndef STOCKDATAUTIL_H
#define STOCKDATAUTIL_H

#include <QString>
#include <QVector>

// 股票数据索引
#define STOCK_DATA_YEAR         0
#define STOCK_DATA_MONTH        1
#define STOCK_DATA_DAY          2
#define STOCK_DATA_HOUR         3
#define STOCK_DATA_HOUR_YI      3
#define STOCK_DATA_HOUR_WU      4
#define STOCK_DATA_HOUR_WEI     5
#define STOCK_DATA_XIAN         6  // 限数据
#define MAX_STOCK_DATA_COUNT    7

#define DATA_FIELD_LENGTH   6

// 筛选条件，支持：禄 权  科  忌 羊 激发字的筛选
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

// 支持18个字及其对应状态的筛选
class FilterConditionV3
{
public:
    // 一宫含的状态
    QVector<QString> m_oneIncludes;

    // 一宫不含的状态
    QVector<QString> m_oneExcludes;

    // 二宫含的状态
    QVector<QString> m_twoIncludes;

    // 二宫不含的状态
    QVector<QString> m_twoExcludes;

public:
    bool isEnable() const
    {
        if (m_oneIncludes.isEmpty() && m_oneExcludes.isEmpty()
                && m_twoIncludes.isEmpty() && m_twoExcludes.isEmpty())
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
    // 行业
    QString m_industryName;

    // 股票名称
    QString m_stockName;

    // 农历时间，年和月的数据才有
    QString m_lunarTime;

    // 公历时间的开始时间戳
    qint64 m_beginTime = 0;

    // 公历时间的结束时间戳（含）
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

// 用来存储日线一条数据
class DayLineData
{
public:
    // 行业
    QString m_industryName;

    // 股票名称
    QString m_stockName;

    // 公历开始时间的时间戳
    qint64 m_beginTime = 0;

    // 公历结束时间的时间戳（含）
    qint64 m_endTime = 0;

    // 开盘
    float m_kaiPan = 0.0f;

    // 涨幅百分比（不含%，10%取10）
    float m_zhangFu = 0.0f;

    // 换手百分比（不含%，10%取10）
    float m_huanShou = 0.0f;

    // 总天数
    int m_totalDay = 1;
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

// 筛选算法：支持18个字及状态的筛选
class StockDataUtilV2
{
public:
    StockDataUtilV2();

public:
    // 检查是否满足条件
    // matchAll True 全宫相关，一宫受四六宫激发，二宫受三五宫激发，False 一宫二宫不受三四五六宫激发
    bool checkIfStockDataOk(StockData stockData, const FilterConditionV3& filterCondition, bool matchAll);

    // 按照算法，对股票数据的一二宫内容进行变换处理
    void transformStockData(const StockData& stockData, QString& oneGong, QString& twoGong, bool matchAll);

    void enableDebugLog() { m_enableDebugLog = true; }

private:
    // 激活有括号的激活字
    void activate(QString gongData[DATA_FIELD_LENGTH], bool matchAll);

    // 待激活字后面是激活值去除
    void removeActivateWordWithKuoHao(QString gongData[DATA_FIELD_LENGTH]);

private:
    // 有效字符
    QString m_validChars;

    // 可激活的字
    QString m_activateChars;

    bool m_enableDebugLog = false;
};

#endif // STOCKDATAUTIL_H
