#include "stockdatautil.h"
#include <QStringList>
#include <QDateTime>
#include <QMap>
#include <QRegularExpression>

static QString YiWord = QString::fromWCharArray(L"巳");
static QString WuWord = QString::fromWCharArray(L"午");
static QString WeiWord = QString::fromWCharArray(L"未");
static QString luWord = QString::fromWCharArray(L"禄");
static QString cunWord = QString::fromWCharArray(L"存");
static QString jiWord = QString::fromWCharArray(L"忌");
static QString keWord = QString::fromWCharArray(L"科");
static QString quanWord = QString::fromWCharArray(L"权");

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
    if (data1.indexOf(QString::fromWCharArray(L"空")) >= 0)
    {
        stockData.m_data[0] += data2;
    }
    if (data2.indexOf(QString::fromWCharArray(L"空")) >= 0)
    {
        stockData.m_data[1] += data1;
    }
    if (data5.indexOf(QString::fromWCharArray(L"空")) >= 0)
    {
        stockData.m_data[4] += data6;
    }
    if (data6.indexOf(QString::fromWCharArray(L"空")) >= 0)
    {
        stockData.m_data[5] += data5;
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
        if (hasWord(word, true, stockData.m_data, m_oneMatches))
        {
            return false;
        }
    }

    // 检查二宫不含，每个字都要满足
    for (int i=0; i<filterCondition.m_twoExclude.length(); i++)
    {
        QString word = filterCondition.m_twoExclude[i];
        if (hasWord(word, false, stockData.m_data, m_twoMatches))
        {
            return false;
        }
    }

    // 检查一宫含，只要一个满足
    bool ok = false;
    for (int i=0; i<filterCondition.m_oneInclude.length(); i++)
    {
        QString word = filterCondition.m_oneInclude[i];
        if (hasWord(word, true, stockData.m_data, m_oneMatches))
        {
            ok = true;
            break;
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
        if (hasWord(word, false, stockData.m_data, m_twoMatches))
        {
            ok = true;
            break;
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

bool StockDataUtil::hasCunWord(bool checkOneGong, QString gongDatas[DATA_FIELD_LENGTH], const QVector<int>&)
{
    QString data = checkOneGong?gongDatas[0]:gongDatas[1];

    // 没有存字，直接返回false
    if (data.indexOf(cunWord) < 0)
    {
        return false;
    }

    // 如果一二宫有双存在，不管是同宫还是分宫，都不作存看
    QString oneTwoGong = gongDatas[0] + gongDatas[1];
    int count = 0;
    for (int i=0; i<oneTwoGong.length(); i++)
    {
        if (oneTwoGong[i] == cunWord)
        {
            count++;
        }
    }
    if (count >= 2)
    {
        return false;
    }

    if (haveWordWithoutKuohao(cunWord, data))
    {
        // 无括号的存，在一二宫遇到有或无括号的禄，都不作存看
        if (gongDatas[0].indexOf(luWord) >= 0 || gongDatas[1].indexOf(luWord) >= 0)
        {
            return false;
        }
        else
        {
            return true;
        }
    }
    else
    {
        // 有括号的存在一二宫无论什么情况都不会被筛出来
        return false;
    }
}

bool StockDataUtil::hasLuWord(bool checkOneGong, QString gongDatas[DATA_FIELD_LENGTH], const QVector<int>& matchIndex)
{
    static QVector<QString> keyWords;
    if (keyWords.empty())
    {
        keyWords.append(QString::fromWCharArray(L"(禄)权"));
        keyWords.append(QString::fromWCharArray(L"(禄)科"));
        keyWords.append(QString::fromWCharArray(L"(禄)忌"));
    }

    QString copyGongDatas[DATA_FIELD_LENGTH];
    for (int i=0; i<DATA_FIELD_LENGTH; i++)
    {
        copyGongDatas[i] = gongDatas[i];
        if (i < 2)
        {
            for (const auto& keyWord : keyWords)
            {
                copyGongDatas[i].replace(keyWord, "");
            }
            static QString lulu = QString::fromWCharArray(L"(禄)禄");
            copyGongDatas[i].replace(lulu, luWord);
        }
    }

    QString data = checkOneGong?copyGongDatas[0]:copyGongDatas[1];

    // 没有禄字，直接返回false
    if (data.indexOf(luWord) < 0)
    {
        return false;
    }

    if (haveWordWithoutKuohao(luWord, data))
    {
        // 禄无括号，遇到一二宫有或无括号的存，都不作禄看
        if (copyGongDatas[0].indexOf(cunWord) >= 0 || copyGongDatas[1].indexOf(cunWord) >= 0)
        {
            return false;
        }
        else
        {
            return true;
        }
    }
    else
    {
        QVector<int> oneTwoIndex;
        oneTwoIndex.push_back(0);
        oneTwoIndex.push_back(1);

        // 禄有括号，遇到一二宫无括号的存，才不作禄看
        if (haveWordWithoutKuohao(cunWord, copyGongDatas, oneTwoIndex))
        {
            return false;
        }

        // 检查是否被其它宫无括号的禄激发
        if (haveWordWithoutKuohao(luWord, copyGongDatas, matchIndex))
        {
            return true;
        }
        else
        {
            return false;
        }
    }
}

bool StockDataUtil::hasJiWord(bool checkOneGong, QString gongDatas[DATA_FIELD_LENGTH], const QVector<int>& matchIndex)
{
    static QVector<QString> keyWords;
    if (keyWords.empty())
    {
        keyWords.append(QString::fromWCharArray(L"阳(忌)忌"));
        keyWords.append(QString::fromWCharArray(L"阳(权)忌"));
        keyWords.append(QString::fromWCharArray(L"阳(科)忌"));
        keyWords.append(QString::fromWCharArray(L"阳(禄)忌"));
        keyWords.append(QString::fromWCharArray(L"阳忌"));
        keyWords.append(QString::fromWCharArray(L"阳(忌)"));
        keyWords.append(QString::fromWCharArray(L"(忌)禄"));
        keyWords.append(QString::fromWCharArray(L"(忌)权"));
        keyWords.append(QString::fromWCharArray(L"(忌)科"));
    }

    QString copyGongDatas[DATA_FIELD_LENGTH];
    for (int i=0; i<DATA_FIELD_LENGTH; i++)
    {
        copyGongDatas[i] = gongDatas[i];
        if (i < 2)
        {
            for (const auto& keyWord : keyWords)
            {
                copyGongDatas[i].replace(keyWord, "");
            }
            static QString jiji = QString::fromWCharArray(L"(忌)忌");
            copyGongDatas[i].replace(jiji, jiWord);
        }
    }

    QString data = checkOneGong?copyGongDatas[0]:copyGongDatas[1];
    if (data.indexOf(jiWord) >= 0 && haveWordWithoutKuohao(jiWord, copyGongDatas, matchIndex))
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool StockDataUtil::hasKeWord(bool checkOneGong, QString gongDatas[DATA_FIELD_LENGTH], const QVector<int>& matchIndex)
{
    static QVector<QString> keyWords;
    if (keyWords.empty())
    {
        keyWords.append(QString::fromWCharArray(L"昌科"));
        keyWords.append(QString::fromWCharArray(L"曲科"));
        keyWords.append(QString::fromWCharArray(L"昌(科)"));
        keyWords.append(QString::fromWCharArray(L"曲(科)"));
        keyWords.append(QString::fromWCharArray(L"(科)禄"));
        keyWords.append(QString::fromWCharArray(L"(科)权"));
        keyWords.append(QString::fromWCharArray(L"(科)忌"));
    }

    QString copyGongDatas[DATA_FIELD_LENGTH];
    for (int i=0; i<DATA_FIELD_LENGTH; i++)
    {
        copyGongDatas[i] = gongDatas[i];
        if (i < 2)
        {
            for (const auto& keyWord : keyWords)
            {
                copyGongDatas[i].replace(keyWord, "");
            }
            static QString keke = QString::fromWCharArray(L"(科)科");
            copyGongDatas[i].replace(keke, keWord);
        }
    }

    QString data = checkOneGong?copyGongDatas[0]:copyGongDatas[1];
    if (data.indexOf(keWord) >= 0 && haveWordWithoutKuohao(keWord, copyGongDatas, matchIndex))
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool StockDataUtil::hasQuanWord(bool checkOneGong, QString gongDatas[DATA_FIELD_LENGTH], const QVector<int>& matchIndex)
{
    static QVector<QString> keyWords;
    if (keyWords.empty())
    {
        keyWords.append(QString::fromWCharArray(L"(权)禄"));
        keyWords.append(QString::fromWCharArray(L"(权)科"));
        keyWords.append(QString::fromWCharArray(L"(权)忌"));
    }

    QString copyGongDatas[DATA_FIELD_LENGTH];
    for (int i=0; i<DATA_FIELD_LENGTH; i++)
    {
        copyGongDatas[i] = gongDatas[i];
        if (i < 2)
        {
            for (const auto& keyWord : keyWords)
            {
                copyGongDatas[i].replace(keyWord, "");
            }
            static QString quanquan = QString::fromWCharArray(L"(权)权");
            copyGongDatas[i].replace(quanquan, quanWord);
        }
    }

    QString data = checkOneGong?copyGongDatas[0]:copyGongDatas[1];
    if (data.indexOf(quanWord) >= 0 && haveWordWithoutKuohao(quanWord, copyGongDatas, matchIndex))
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool StockDataUtil::hasWord(const QString& word, bool checkOneGong, QString gongDatas[DATA_FIELD_LENGTH], const QVector<int>& matchIndex)
{
    if (word == cunWord)
    {
        return hasCunWord(checkOneGong, gongDatas, matchIndex);
    }
    else if (word == luWord)
    {
        return hasLuWord(checkOneGong, gongDatas, matchIndex);
    }
    else if (word == jiWord)
    {
        return hasJiWord(checkOneGong, gongDatas, matchIndex);
    }
    else if (word == keWord)
    {
        return hasKeWord(checkOneGong, gongDatas, matchIndex);
    }
    else if (word == quanWord)
    {
        return hasQuanWord(checkOneGong, gongDatas, matchIndex);
    }
    else
    {
        QString data = checkOneGong?gongDatas[0]:gongDatas[1];
        if (data.indexOf(word) >= 0 && haveWordWithoutKuohao(word, gongDatas, matchIndex))
        {
            return true;
        }
        else
        {
            return false;
        }
    }
}

bool StockDataUtil::haveWordWithoutKuohao(const QString& word, const QString& data)
{
    for (int i=0; i<data.length(); i++)
    {
        if (data[i] == word)
        {
            if (i == 0 || data[i-1] != "(" || i == data.length() - 1
                    || data[i+1] != ")")
            {
                return true;
            }
        }
    }

    return false;
}

bool StockDataUtil::haveWordWithoutKuohao(const QString& word, QString data[DATA_FIELD_LENGTH], const QVector<int>& matchIndex)
{
    for (int t=0; t<matchIndex.size(); t++)
    {
        int j = matchIndex[t];
        if (haveWordWithoutKuohao(word, data[j]))
        {
            return true;
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
    if (dataType == STOCK_DATA_YEAR || dataType == STOCK_DATA_MONTH || dataType == STOCK_DATA_XIAN)
    {
        if (newFields.length() != 9)
        {
            return false;
        }

        stockData.m_lunarTime = newFields[0];

        if (dataType == STOCK_DATA_XIAN)
        {
            QDateTime beginTime = QDateTime::fromString(newFields[1], "yyyy");
            if (!beginTime.isValid())
            {
                return false;
            }
            stockData.m_beginTime = beginTime.toSecsSinceEpoch();

            QDateTime endTime = QDateTime::fromString(newFields[2], "yyyy");
            if (!endTime.isValid())
            {
                return false;
            }
            stockData.m_endTime = endTime.addYears(1).toSecsSinceEpoch()-1;
        }
        else
        {
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
            stockData.m_endTime = endTime.addDays(1).toSecsSinceEpoch()-1;
        }

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
        stockData.m_endTime = stockData.m_beginTime + 24*3600 -1;

        for (int i=0; i<DATA_FIELD_LENGTH; i++)
        {
            stockData.m_data[i] = newFields[1+i];
        }
    }

    return true;
}

StockDataUtilV2::StockDataUtilV2()
{
    m_validChars = QString::fromWCharArray(L"()阴阳武破机紫府狼巨廉相梁杀同羊存昌曲禄权科忌");
    m_activateChars = QString::fromWCharArray(L"禄权科忌羊");
}

bool StockDataUtilV2::checkIfStockDataOk(StockData stockData, const FilterConditionV3& filterCondition, bool matchAll)
{
    if (!filterCondition.isEnable())
    {
        return false;
    }

    // 根据算法重新调整一二宫的内容
    QString oneGong;
    QString twoGong;
    transformStockData(stockData, oneGong, twoGong, matchAll);

    // 一宫不含，都要满足
    for (const auto& status : filterCondition.m_oneExcludes)
    {
        if (oneGong.indexOf(status) >= 0)
        {
            return false;
        }
    }

    // 二宫不含，都要满足
    for (const auto& status : filterCondition.m_twoExcludes)
    {
        if (twoGong.indexOf(status) >= 0)
        {
            return false;
        }
    }

    // 一宫含，只要含一个就满足
    bool ok = false;
    for (const auto& status : filterCondition.m_oneIncludes)
    {
        if (oneGong.indexOf(status) >= 0)
        {
            ok = true;
            break;
        }
    }

    if (!ok)
    {
        return false;
    }

    // 二宫含，只要含一个就满足
    ok = false;
    for (const auto& status : filterCondition.m_twoIncludes)
    {
        if (twoGong.indexOf(status) >= 0)
        {
            ok = true;
            break;
        }
    }

    if (!ok)
    {
        return false;
    }

    return true;
}

void StockDataUtilV2::transformStockData(const StockData& stockData, QString& oneGong, QString& twoGong, bool matchAll)
{
    QString gongData[DATA_FIELD_LENGTH];
    for (int i=0; i<DATA_FIELD_LENGTH; i++)
    {
        gongData[i] = stockData.m_data[i];
    }

    // 一二宫有空互相借位，五六宫有空互相借位
    if (gongData[0].indexOf(QString::fromWCharArray(L"空")) >= 0)
    {
        gongData[0].replace(QString::fromWCharArray(L"空"), stockData.m_data[1]);
    }

    if (gongData[1].indexOf(QString::fromWCharArray(L"空")) >= 0)
    {
        gongData[1].replace(QString::fromWCharArray(L"空"), stockData.m_data[0]);
    }

    if (gongData[4].indexOf(QString::fromWCharArray(L"空")) >= 0)
    {
        gongData[4].replace(QString::fromWCharArray(L"空"), stockData.m_data[5]);
    }

    if (gongData[5].indexOf(QString::fromWCharArray(L"空")) >= 0)
    {
        gongData[5].replace(QString::fromWCharArray(L"空"), stockData.m_data[4]);
    }

    // 去除无效的字符
    for (int i=0; i<=1; i++)
    {
        QString validChars;
        for (int j=0; j<gongData[i].length(); j++)
        {
            if (m_validChars.indexOf(gongData[i][j]) >= 0)
            {
                validChars.append(gongData[i][j]);
            }
        }
        gongData[i] = validChars;
    }

    // 待激活字后面是激活值字时去除，比如：(权)科 => 科，即使后面有权激活
    for (int i=0; i<=1; i++)
    {
        QRegularExpression regex(QString::fromWCharArray(L"\\((禄|权|科|忌)\\)(禄|权|科|忌)"));
        gongData[i].replace(regex, "\\2");
    }

    // 有括号被无括号的激活字激活
    activate(gongData, matchAll);

    // 一些特殊字的替换
    for (int i=0; i<=1; i++)
    {
        gongData[i].replace(QString::fromWCharArray(L"(昌)科"), QString::fromWCharArray(L"昌科"));
        gongData[i].replace(QString::fromWCharArray(L"(昌)忌"), QString::fromWCharArray(L"昌忌"));
        gongData[i].replace(QString::fromWCharArray(L"(曲)科"), QString::fromWCharArray(L"曲科"));
        gongData[i].replace(QString::fromWCharArray(L"(曲)忌"), QString::fromWCharArray(L"曲忌"));
    }

    // 去除所有括号及括号的内容
    for (int i=0; i<=1; i++)
    {
        QRegularExpression regex("\\([^()]*\\)");
        gongData[i].replace(regex, "");
    }

    if (m_enableDebugLog)
    {
        QString allData;
        for (int i=0; i<DATA_FIELD_LENGTH; i++)
        {
            allData += stockData.m_data[i] + ", ";
        }

        QString newData = gongData[0] + ", " + gongData[1];

        qDebug("old data: %s   new data: %s", allData.toStdString().c_str(), newData.toStdString().c_str());
    }

    oneGong = gongData[0];
    twoGong = gongData[1];
}

void StockDataUtilV2::activate(QString gongData[DATA_FIELD_LENGTH], bool matchAll)
{
    for (int i=0; i<=1; i++)
    {
        for (const auto& activateChar : m_activateChars)
        {
            if (gongData[i].indexOf(QString("(")+activateChar+")") < 0)
            {
                continue;
            }

            // 查找目标字符，且后面不是 ) 符号
            QRegularExpression regex(QString("%1(?!\\))").arg(activateChar));
            bool match = regex.match(gongData[0]).hasMatch() || regex.match(gongData[1]).hasMatch();
            if (!match)
            {
                if (matchAll && i==0)
                {
                    // 一宫受四六宫激发
                    match = regex.match(gongData[3]).hasMatch() || regex.match(gongData[5]).hasMatch();
                }
                else if (matchAll && i==1)
                {
                    // 二宫受三五宫激发
                    match = regex.match(gongData[2]).hasMatch() || regex.match(gongData[4]).hasMatch();
                }
            }

            if (match)
            {
                gongData[i].replace(QString("(")+activateChar+")", activateChar);
            }
        }
    }
}
