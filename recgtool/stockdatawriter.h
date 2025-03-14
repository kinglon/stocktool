#ifndef STOCKDATAWRITER_H
#define STOCKDATAWRITER_H

#include <QString>
#include "stockdatamanager.h"

class StockScore
{
public:
    // 行业
    QString m_industry;

    // 股票名
    QString m_stockName;

    // 得分总分
    float m_score = 0.0f;

    // 该股票匹配到的天数
    int m_totalDay = 0;

public:
    float getAvarageScore() const
    {
        if (m_totalDay == 0)
        {
            return 0.0f;
        }

        return m_score / m_totalDay;
    }
};

class StockDataWriter
{
public:
    StockDataWriter();

    static QString getResult(QVector<StockData>& stockDatas);

    static bool saveResult(const QString& savePath);

private:
    static QVector<StockScore> calcStockScore(const QVector<StockData>& stockDatas);

    static void writeSpaces(int count, QString& result);

    static void writeStockXianContent(const StockData& stockData, QString& result);

    // 追加一行限内容
    static void writeStockXianContent2(const StockData& xianData, QString& result);

    static void writeStockYearContent(const StockData& stockData, QString& result);

    static void writeStockMonthContent(const StockData& stockData, QString& result);

    static void writeStockDayContent(const StockData& stockData, QString& result);
};

#endif // STOCKDATAWRITER_H
