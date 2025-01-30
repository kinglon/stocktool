#include "stockdatautil.h"
#include <QStringList>
#include <QDateTime>

QString YiWord = QString::fromWCharArray(L"巳");
QString WuWord = QString::fromWCharArray(L"午");
QString WeiWord = QString::fromWCharArray(L"未");

bool StockDataUtil::checkIfStockDataOk(StockData stockData, const FilterCondition& filterCondition, bool matchAll)
{
    if (!filterCondition.isEnable())
    {
        return true;
    }

    // 一二宫有空先借位，五六宫有空先借位
    QString data1 = stockData.m_data[0];
    QString data2 = stockData.m_data[1];
    QString data5 = stockData.m_data[4];
    QString data6 = stockData.m_data[5];
    if (stockData.m_data[0].indexOf(QString::fromWCharArray(L"空")) >= 0)
    {
        stockData.m_data[0] += data2;
    }
    if (stockData.m_data[1].indexOf(QString::fromWCharArray(L"空")) >= 0)
    {
        stockData.m_data[1] += data1;
    }
    if (stockData.m_data[4].indexOf(QString::fromWCharArray(L"空")) >= 0)
    {
        stockData.m_data[4] += data6;
    }
    if (stockData.m_data[5].indexOf(QString::fromWCharArray(L"空")) >= 0)
    {
        stockData.m_data[5] += data5;
    }

    // 删除：阳忌、昌科、曲科、阳（忌）、昌（科）、曲（科）
    for (int i=0; i<DATA_FIELD_LENGTH; i++)
    {
        stockData.m_data[i].replace(QString::fromWCharArray(L"阳忌"), "");
        stockData.m_data[i].replace(QString::fromWCharArray(L"昌科"), "");
        stockData.m_data[i].replace(QString::fromWCharArray(L"曲科"), "");
        stockData.m_data[i].replace(QString::fromWCharArray(L"阳(忌)"), "");
        stockData.m_data[i].replace(QString::fromWCharArray(L"昌(科)"), "");
        stockData.m_data[i].replace(QString::fromWCharArray(L"曲(科)"), "");
    }

    // 一二宫有存字，就删除禄字
    if (data1.indexOf(QString::fromWCharArray(L"存")) >= 0 ||
            data2.indexOf(QString::fromWCharArray(L"存")) >= 0)
    {
        stockData.m_data[0].replace(QString::fromWCharArray(L"禄"), "");
        stockData.m_data[1].replace(QString::fromWCharArray(L"禄"), "");
    }

    // 一二宫有禄字，就删除存字
    if (data1.indexOf(QString::fromWCharArray(L"禄")) >= 0 ||
            data2.indexOf(QString::fromWCharArray(L"禄")) >= 0)
    {
        stockData.m_data[0].replace(QString::fromWCharArray(L"存"), "");
        stockData.m_data[1].replace(QString::fromWCharArray(L"存"), "");
    }

    // matchAll True 全宫相关匹配，False 一宫不与四六宫匹配，二宫不与三五宫匹配
    QVector<int> m_oneMatches;
    QVector<int> m_twoMatches;
    for (int i=0; i<DATA_FIELD_LENGTH; i++)
    {
        if (matchAll || (i != 3 && i != 5))
        {
            m_oneMatches.append(i);
        }

        if (matchAll || (i != 2 && i != 4))
        {
            m_twoMatches.append(i);
        }
    }

    // 检查一宫不含，每个字都要满足
    for (int i=0; i<filterCondition.m_oneExclude.length(); i++)
    {
        QString word = filterCondition.m_oneExclude[i];
        if (word == QString::fromWCharArray(L"存"))
        {
            if (hasCunWord(stockData.m_data[0], stockData.m_data[0], stockData.m_data[1]))
            {
                return false;
            }
        }
        else
        {
            if (stockData.m_data[0].indexOf(word) >= 0 && haveWordWithoutKuohao(word, stockData.m_data, m_oneMatches))
            {
                return false;
            }
        }
    }

    // 检查二宫不含，每个字都要满足
    for (int i=0; i<filterCondition.m_twoExclude.length(); i++)
    {
        QString word = filterCondition.m_twoExclude[i];
        if (word == QString::fromWCharArray(L"存"))
        {
            if (hasCunWord(stockData.m_data[1], stockData.m_data[0], stockData.m_data[1]))
            {
                return false;
            }
        }
        else
        {
            if (stockData.m_data[1].indexOf(word) >= 0 && haveWordWithoutKuohao(word, stockData.m_data, m_twoMatches))
            {
                return false;
            }
        }
    }

    // 检查一宫含，只要一个满足
    bool ok = false;
    for (int i=0; i<filterCondition.m_oneInclude.length(); i++)
    {
        QString word = filterCondition.m_oneInclude[i];
        if (word == QString::fromWCharArray(L"存"))
        {
            if (hasCunWord(stockData.m_data[0], stockData.m_data[0], stockData.m_data[1]))
            {
                ok = true;
                break;
            }
        }
        else
        {
            if (stockData.m_data[0].indexOf(word) >= 0 && haveWordWithoutKuohao(word, stockData.m_data, m_oneMatches))
            {
                ok = true;
                break;
            }
        }
    }
    if (filterCondition.m_oneInclude.length() > 0 && !ok)
    {
        return false;
    }

    // 检查二宫含，只要一个满足
    ok = false;
    for (int i=0; i<filterCondition.m_twoInclude.length(); i++)
    {
        QString word = filterCondition.m_twoInclude[i];
        if (word == QString::fromWCharArray(L"存"))
        {
            if (hasCunWord(stockData.m_data[1], stockData.m_data[0], stockData.m_data[1]))
            {
                ok = true;
                break;
            }
        }
        else
        {
            if (stockData.m_data[1].indexOf(word) >= 0 && haveWordWithoutKuohao(word, stockData.m_data, m_twoMatches))
            {
                ok = true;
                break;
            }
        }
    }
    if (filterCondition.m_twoInclude.length() > 0 && !ok)
    {
        return false;
    }

    return true;
}

bool StockDataUtil::checkIfStockDataOkV2(StockData stockData, const FilterConditionV2& filterCondition)
{
    if (!filterCondition.isEnable())
    {
        return true;
    }

    for (int i=0; i<DATA_FIELD_LENGTH; i++)
    {
        if (!filterCondition.m_filterItems[i].m_include.isEmpty()
                && stockData.m_data[i].indexOf(filterCondition.m_filterItems[i].m_include) < 0)
        {
            return false;
        }

        if (!filterCondition.m_filterItems[i].m_exclude.isEmpty()
                && stockData.m_data[i].indexOf(filterCondition.m_filterItems[i].m_exclude) >= 0)
        {
            return false;
        }
    }

    return true;
}

bool StockDataUtil::hasCunWord(QString data, QString data1, QString data2)
{
    QString cun = QString::fromWCharArray(L"存");

    // 只能有一个存字
    int count = 0;
    for (int i=0; i<data.length(); i++)
    {
        if (data[i] == cun)
        {
            count++;
        }
    }
    if (count != 1)
    {
        return false;
    }

    // 存前后是括号，返回false
    if (data.indexOf(QString::fromWCharArray(L"(存)")) >= 0)
    {
        return false;
    }

    // 一二宫带有禄字，返回false
    if (data1.indexOf(QString::fromWCharArray(L"禄")) >= 0 ||
            data2.indexOf(QString::fromWCharArray(L"禄")) >= 0)
    {
        return false;
    }

    return true;
}

bool StockDataUtil::haveWordWithoutKuohao(QString word, QString data[DATA_FIELD_LENGTH], QVector<int> matchIndex)
{
    for (int t=0; t<matchIndex.size(); t++)
    {
        int j = matchIndex[t];
        QString currentData = data[j];
        for (int i=0; i<currentData.length(); i++)
        {
            if (currentData[i] == word)
            {
                if (i == 0 || currentData[i-1] != "(" || i == currentData.length() - 1
                        || currentData[i+1] != ")")
                {
                    return true;
                }
            }
        }
    }

    return false;
}

bool StockDataUtil::parseOneLine(const QString& industryName, const QString& stockName, int dataType, const QString& line, StockData& stockData)
{
    if (line.isEmpty())
    {
        return false;
    }

    QStringList fields = line.split(',');
    if (fields.length() < 7) // 至少要有7个字段，时数据字段最少
    {
        return false;
    }

    QStringList newFields;
    for (auto& field : fields)
    {
        // 去除前后双引号
        QString subString = field.mid(1, field.length()-2);
        if (subString.isEmpty())
        {
            return false;
        }
        newFields.append(subString);
    }

    if (newFields[0] == QString::fromWCharArray(L"时间"))
    {
        return false;
    }

    stockData.m_industryName = industryName;
    stockData.m_stockName = stockName;
    if (dataType == STOCK_DATA_YEAR || dataType == STOCK_DATA_MONTH)
    {
        if (newFields.length() != 9)
        {
            return false;
        }

        stockData.m_lunarTime = newFields[0];

        QDateTime beginTime = QDateTime::fromString(newFields[1], "yyyy-M-d");
        if (!beginTime.isValid())
        {
            return false;
        }
        stockData.m_beginTime = beginTime.toSecsSinceEpoch();

        QDateTime endTime = QDateTime::fromString(newFields[2], "yyyy-M-d");
        if (!endTime.isValid())
        {
            return false;
        }
        stockData.m_endTime = endTime.toSecsSinceEpoch();

        for (int i=0; i<DATA_FIELD_LENGTH; i++)
        {
            stockData.m_data[i] = newFields[3+i];
        }
    }
    else
    {
        if (newFields.length() != 7)
        {
            return false;
        }

        // 时数据，有大量不需要，优先过滤掉
        QString dateString;
        if (dataType == STOCK_DATA_HOUR)
        {
            stockData.m_hour = newFields[0].right(1);
            if (stockData.m_hour != YiWord && stockData.m_hour != WuWord && stockData.m_hour != WeiWord)
            {
                return false;
            }
            dateString = newFields[0].left(newFields[0].length()-1);
        }
        else
        {
            dateString = newFields[0];
        }

        QDateTime dateTime = QDateTime::fromString(dateString, QString::fromWCharArray(L"yyyy年 M月 d日"));
        stockData.m_beginTime = dateTime.toSecsSinceEpoch();
        stockData.m_endTime = stockData.m_beginTime;

        for (int i=0; i<DATA_FIELD_LENGTH; i++)
        {
            stockData.m_data[i] = newFields[1+i];
        }
    }

    return true;
}
