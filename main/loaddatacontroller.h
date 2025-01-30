#ifndef LOADDATACONTROLLER_H
#define LOADDATACONTROLLER_H

#include <QObject>
#include <QThread>
#include "datamodel.h"
#include "datamanager.h"

class StockFileScanner: public QObject
{
    Q_OBJECT
public:
    explicit StockFileScanner(QObject *parent = nullptr);

public:
    void run();    

signals:
    void runFinish();

public:
    // 扫描根目录
    QString m_rootDir;

    // 扫描的股票文件完整路径
    QVector<QString> m_stockFiles;

    // 行业名称，与m_stockFiles一一对应
    QVector<QString> m_industryNames;
};

class StockFileLoader: public QObject
{
    Q_OBJECT
public:
    explicit StockFileLoader(QObject *parent = nullptr);

public:
    void run();

signals:
    // 每10万条数据报告一次
    void reportProgress(int count);

    void runFinish(StockFileLoader* loader);

private:
    bool getStockNameAndType(const QString& fileName, QString& stockName, int& type);

    void processOneLine(const QString& industryName, const QString& stockName, int dataType, const QString& line);

public:
    QVector<QString> m_stockFiles;

    QVector<QString> m_industryNames;

    QVector<StockData> m_stockDatas[MAX_STOCK_DATA_COUNT];

    int m_count = 0;
};

class LoadDataController : public QObject
{
    Q_OBJECT
public:
    explicit LoadDataController(QObject *parent = nullptr);

public:
    void run(QString rootDir);

signals:
    void printLog(QString content);    

    void runFinish();

private slots:
    void onStockFileScannerFinish();

    void onReportProgress(int count);

    void onStockFileLoaderFinish(StockFileLoader* loader);

private:
    StockFileScanner* m_stockFileScanner = nullptr;

    QThread* m_stockFileScanThread = nullptr;

    QVector<QThread*> m_loadDataThreads;

    QVector<StockFileLoader*> m_stockFileLoaders;

    int m_totalCount = 0;
};

#endif // LOADDATACONTROLLER_H
