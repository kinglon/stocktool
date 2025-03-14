#ifndef MAINCONTROLLER_H
#define MAINCONTROLLER_H

#include <QObject>
#include <QThread>
#include "settingmanager.h"

class WriteDataThread : public QThread
{
    Q_OBJECT

protected:
    void run() override;

signals:
    void runFinish();

public:
    QVector<StockData> m_stockDatas;
};

class MainController : public QObject
{
    Q_OBJECT
public:
    explicit MainController(QObject *parent = nullptr);

    void run();

signals:
    void runFinish();

    void printLog(QString content);

    void printResult(QString result);

private:
    void doLoadData();

    void doFilterData();

    void doWriteData(const QVector<StockData>& stockDatas);

private slots:
    void onWriteDataFinish();

private:
    int m_loadDataNextIndex = STOCK_TYPE_GEGU;
};

#endif // MAINCONTROLLER_H
