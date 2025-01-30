#include "contentbrowser.h"
#include <QThread>
#include <QTimer>
#include <QDir>
#include <QTextStream>
#include <QDateTime>
#include "Utility/ImPath.h"
#include <QFileInfo>
#include "settingmanager.h"

#define STOCK_DATA_YEAR         0
#define STOCK_DATA_MONTH        1
#define STOCK_DATA_DAY          2
#define STOCK_DATA_HOUR         3
#define STOCK_DATA_HOUR_YI      3
#define STOCK_DATA_HOUR_WU      4
#define STOCK_DATA_HOUR_WEI     5
#define MAX_STOCK_DATA_COUNT    6

ContentBrowser::ContentBrowser(QObject *parent)
    : QObject{parent}
{

}

void ContentBrowser::run()
{
    QDir dir(m_stockPath);
    m_stockName = dir.dirName();

    QString content;
    printLimitContent(content);
    printYearContent(content);
    printMonthContent(content);
    printDayContent(content);
    printHourContent(content);

    if (!content.isEmpty())
    {
        emit printContent(content);
    }
    else
    {
        emit printContent(QString::fromWCharArray(L"无数据"));
    }
    emit runFinish();
}

void ContentBrowser::printLimitContent(QString& content)
{
    QString stockDataFilePath = m_stockPath + "\\" + m_stockName + QString::fromWCharArray(L"限.csv");
    QFile file(stockDataFilePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        return;
    }

    int thisYear = QDate::currentDate().year();
    QTextStream in(&file);
    while (!in.atEnd())
    {
        QString line = in.readLine();
        if (line.isEmpty())
        {
            continue;
        }

        QStringList fields = line.split(',');
        QStringList newFields;
        for (auto& field : fields)
        {
            // 去除前后双引号
            QString subString = field.mid(1, field.length()-2);
            if (subString.isEmpty())
            {
                continue;
            }
            newFields.append(subString);
        }

        if (newFields.size() < 3)
        {
            continue;
        }

        if (newFields[0] == QString::fromWCharArray(L"时间"))
        {
            continue;
        }

        bool needOutput = false;
        if (newFields[0] == QString::fromWCharArray(L"1岁大限"))
        {
            needOutput = true;
        }
        else
        {
            bool ok = false;
            int beginYear = newFields[1].toInt(&ok);
            if (!ok)
            {
                continue;
            }

            int endYear = newFields[2].toInt(&ok);
            if (!ok)
            {
                continue;
            }

            if (thisYear >= beginYear && thisYear <= endYear)
            {
                needOutput = true;
            }
        }

        if (needOutput)
        {
            content += "   ";
            for (const auto& field : newFields)
            {
                content += field + " ";
            }
            content += "\n";
        }
    }
    file.close();

    content += "\n\n";
}

void ContentBrowser::printYearContent(QString& content)
{
    QDateTime begin, end;
    begin.setDate(QDate(m_originDate.year()-SettingManager::getInstance()->m_beforeYear, 1, 1));
    qint64 beginTime = begin.toSecsSinceEpoch();
    end.setDate(QDate(m_originDate.year()+1+SettingManager::getInstance()->m_afterYear, 1, 1));
    qint64 endTime = end.toSecsSinceEpoch();
    QString stockDataFilePath = m_stockPath + "\\" + m_stockName + QString::fromWCharArray(L"年.csv");
    QVector<StockData> stockDatas = loadStockData(stockDataFilePath, STOCK_DATA_YEAR, endTime);
    if (stockDatas.empty())
    {
        return;
    }

    for (const auto& stockData : stockDatas)
    {
        if (stockData.m_beginTime >= endTime)
        {
            break;
        }

        if (stockData.m_beginTime >= beginTime)
        {
            content += "   " + stockData.m_lunarTime;
            for (int i=0; i<DATA_FIELD_LENGTH; i++)
            {
                content += " " + stockData.m_data[i];
            }
            content += "\n";
        }
    }
    content += "\n\n";
}

void ContentBrowser::printMonthContent(QString& content)
{
    QDateTime begin, end;
    begin.setDate(QDate(m_originDate.year(), m_originDate.month(), 1).addMonths(-1*SettingManager::getInstance()->m_beforeMonth));
    qint64 beginTime = begin.toSecsSinceEpoch();
    end.setDate(QDate(m_originDate.year(), m_originDate.month(), 1).addMonths(SettingManager::getInstance()->m_afterMonth+1));
    qint64 endTime = end.toSecsSinceEpoch();

    QString stockDataFilePath = m_stockPath + "\\" + m_stockName + QString::fromWCharArray(L"月.csv");
    QVector<StockData> stockDatas = loadStockData(stockDataFilePath, STOCK_DATA_MONTH, endTime);
    if (stockDatas.empty())
    {
        return;
    }

    for (const auto& stockData : stockDatas)
    {
        if (stockData.m_beginTime >= endTime)
        {
            break;
        }

        if (stockData.m_beginTime >= beginTime)
        {
            content += "   " + stockData.m_lunarTime;
            for (int i=0; i<DATA_FIELD_LENGTH; i++)
            {
                content += " " + stockData.m_data[i];
            }
            content += "\n";
        }
    }
    content += "\n\n";
}

void ContentBrowser::printDayContent(QString& content)
{
    QDateTime begin, end;
    begin.setDate(m_originDate.addDays(-1*SettingManager::getInstance()->m_beforeDay));
    qint64 beginTime = begin.toSecsSinceEpoch();
    end.setDate(m_originDate.addDays(SettingManager::getInstance()->m_afterDay+1));
    qint64 endTime = end.toSecsSinceEpoch();

    QString stockDataFilePath = m_stockPath + "\\" + m_stockName + QString::fromWCharArray(L"日.csv");
    QVector<StockData> stockDatas = loadStockData(stockDataFilePath, STOCK_DATA_DAY, endTime);
    if (stockDatas.empty())
    {
        return;
    }

    for (const auto& stockData : stockDatas)
    {
        if (stockData.m_beginTime >= endTime)
        {
            break;
        }

        if (stockData.m_beginTime >= beginTime)
        {
            content += "   " + QDateTime::fromSecsSinceEpoch(stockData.m_beginTime).toString(QString::fromWCharArray(L"yyyy年 M月 d日"));
            for (int i=0; i<DATA_FIELD_LENGTH; i++)
            {
                content += " " + stockData.m_data[i];
            }
            content += "\n";
        }
    }
}

void ContentBrowser::printHourContent(QString& content)
{
    QDateTime begin, end;
    begin.setDate(m_originDate.addDays(-1*SettingManager::getInstance()->m_beforeHour));
    qint64 beginTime = begin.toSecsSinceEpoch();
    end.setDate(m_originDate.addDays(SettingManager::getInstance()->m_afterHour+1));
    qint64 endTime = end.toSecsSinceEpoch();

    QString stockDataFilePath = m_stockPath + "\\" + m_stockName + QString::fromWCharArray(L"时.csv");
    QVector<StockData> stockDatas = loadStockData(stockDataFilePath, STOCK_DATA_HOUR, endTime);
    if (stockDatas.empty())
    {
        return;
    }

    qint64 currentTime = 0;
    for (const auto& stockData : stockDatas)
    {
        if (stockData.m_beginTime >= endTime)
        {
            break;
        }

        if (stockData.m_beginTime >= beginTime)
        {
            if (stockData.m_beginTime != currentTime)
            {
                currentTime = stockData.m_beginTime;
                content += QString("\n\n") + "   " + QDateTime::fromSecsSinceEpoch(currentTime).toString(QString::fromWCharArray(L"yyyy年 M月 d日")) + "\n";
            }
            content += "   " + QDateTime::fromSecsSinceEpoch(stockData.m_beginTime).toString(QString::fromWCharArray(L"yyyy年 M月 d日")) + stockData.m_hour;
            for (int i=0; i<DATA_FIELD_LENGTH; i++)
            {
                content += " " + stockData.m_data[i];
            }
            content += "\n";
        }
    }
}

QVector<StockData> ContentBrowser::loadStockData(const QString& stockDataFilePath, int dataType, qint64 endTime)
{
    QVector<StockData> stockDatas;

    QFile file(stockDataFilePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        return stockDatas;
    }

    QTextStream in(&file);
    while (!in.atEnd())
    {
        QString line = in.readLine();
        StockData stockData;
        if (!StockDataUtil::parseOneLine("", m_stockName, dataType, line, stockData))
        {
            continue;
        }

        if (stockData.m_beginTime >= endTime)
        {
            break;
        }

        stockDatas.append(stockData);
    }
    file.close();

    return stockDatas;
}
