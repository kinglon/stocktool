#ifndef CONTENTBROWSER_H
#define CONTENTBROWSER_H

#include <QObject>
#include <QThread>
#include <QDate>
#include "datamodel.h"
#include "../main/stockdatautil.h"

class ContentBrowser: public QObject
{
    Q_OBJECT
public:
    explicit ContentBrowser(QObject *parent = nullptr);

public:
    void run();

signals:
    // 打印内容
    void printContent(QString content);

    void runFinish();

private:
    // 打印股票限.csv的内容
    void printLimitContent(QString& content);

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
