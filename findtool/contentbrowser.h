#ifndef CONTENTBROWSER_H
#define CONTENTBROWSER_H

#include <QObject>
#include <QThread>
#include <QDate>
#include "datamodel.h"

class ContentBrowser: public QObject
{
    Q_OBJECT
public:
    explicit ContentBrowser(QObject *parent = nullptr);

public:
    void run();

    static bool parseOneLine(const QString& industryName, const QString& stockName, int dataType, const QString& line, StockData& stockData);

signals:
    // 打印内容
    void printContent(QString content);

    void runFinish();

private:
    void printYearContent(QString& content);

    void printMonthContent(QString& content);

    void printDayContent(QString& content);

    void printHourContent(QString& content);

    QVector<StockData> loadStockData(const QString& stockDataFilePath, int dataType, qint64 endTime);

public:
    QString m_stockName;

    QString m_stockPath;

    QDate m_originDate;
};

#endif // CONTENTBROWSER_H
