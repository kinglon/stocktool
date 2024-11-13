#include "loaddatacontroller.h"
#include <QThread>
#include <QTimer>
#include <QDir>
#include <QTextStream>
#include <QDateTime>
#include "Utility/ImPath.h"

QString YiWord = QString::fromWCharArray(L"巳");
QString WuWord = QString::fromWCharArray(L"午");
QString WeiWord = QString::fromWCharArray(L"未");

StockFileScanner::StockFileScanner(QObject *parent)
    : QObject{parent}
{

}

void StockFileScanner::run()
{    
    QDir dir(m_rootDir);
    QDir::Filters filters = QDir::Dirs | QDir::NoDotAndDotDot;
    QFileInfoList fileInfoList = dir.entryInfoList(filters);
    QStringList suffixes;
    suffixes.append(QString::fromWCharArray(L"年.csv"));
    suffixes.append(QString::fromWCharArray(L"月.csv"));
    suffixes.append(QString::fromWCharArray(L"日.csv"));
    suffixes.append(QString::fromWCharArray(L"时.csv"));
    foreach (const QFileInfo &fileInfo, fileInfoList)
    {
        bool found = false;
        foreach(const QString& suffix, suffixes)
        {
            QString filePath = fileInfo.absoluteFilePath() + "\\" + fileInfo.fileName() + suffix;
            if (QFile(filePath).exists())
            {
                found = true;
                m_stockFiles.append(filePath);
                m_industryNames.append("");
            }
        }

        // 行业下加载
        if (!found)
        {
            QDir subDir(fileInfo.absoluteFilePath());
            QFileInfoList subFileInfoList = subDir.entryInfoList(filters);
            foreach (const QFileInfo &subFileInfo, subFileInfoList)
            {
                foreach(const QString& suffix, suffixes)
                {
                    QString filePath = subFileInfo.absoluteFilePath() + "\\" + subFileInfo.fileName() + suffix;
                    if (QFile(filePath).exists())
                    {
                        m_stockFiles.append(filePath);
                        m_industryNames.append(fileInfo.fileName());
                    }
                }
            }
        }
    }
    emit runFinish();
}

StockFileLoader::StockFileLoader(QObject *parent)
    : QObject{parent}
{

}

void StockFileLoader::run()
{
    for(int i=0; i<m_stockFiles.size(); i++)
    {
        const QString& stockFile = m_stockFiles[i];
        const QString& industryName = m_industryNames[i];
        QFileInfo fileInfo(stockFile);
        QString fileName = fileInfo.fileName();
        QString stockName;
        int dataType = -1;
        if (!getStockNameAndType(fileName, stockName, dataType))
        {
            continue;
        }

        QFile file(stockFile);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            qCritical("failed to open file: %s", stockFile.toStdString().c_str());
            continue;
        }
        QTextStream in(&file);
        while (!in.atEnd())
        {
            QString line = in.readLine();
            processOneLine(industryName, stockName, dataType, line);
        }
        file.close();
    }
    emit runFinish(this);
}

bool StockFileLoader::getStockNameAndType(const QString& fileName, QString& stockName, int& type)
{
    int suffixLength = 5;  //尾部“年.csv”的长度
    if (fileName.length() <= suffixLength)
    {
        qCritical("wrong file name: %s", fileName.toStdString().c_str());
        return false;
    }

    stockName = fileName.left(fileName.length()-5);  // 5是尾部“年.csv”的长度
    QString typeChar = fileName[stockName.length()];

    QVector<QString> types;
    types.append(QString::fromWCharArray(L"年"));
    types.append(QString::fromWCharArray(L"月"));
    types.append(QString::fromWCharArray(L"日"));
    types.append(QString::fromWCharArray(L"时"));
    type = -1;
    for (int i=0; i<types.size(); i++)
    {
        if (typeChar == types[i])
        {
            type = i;
            break;
        }
    }
    if (type == -1)
    {
        qCritical("wrong file name: %s", fileName.toStdString().c_str());
        return false;
    }

    return true;
}

void StockFileLoader::processOneLine(const QString& industryName, const QString& stockName, int dataType, const QString& line)
{
    if (line.isEmpty())
    {
        return;
    }

    QStringList fields = line.split(',');
    if (fields.length() < 5)
    {
        return;
    }

    QStringList newFields;
    for (auto& field : fields)
    {
        // 去除前后双引号
        QString subString = field.mid(1, field.length()-2);
        if (subString.isEmpty())
        {
            return;
        }
        newFields.append(subString);
    }

    if (newFields[0] == QString::fromWCharArray(L"时间"))
    {
        return;
    }

    StockData stockData;
    stockData.m_industryName = industryName;
    stockData.m_stockName = stockName;
    if (dataType == STOCK_DATA_YEAR || dataType == STOCK_DATA_MONTH)
    {
        if (newFields.length() != 7)
        {
            return;
        }

        stockData.m_lunarTime = newFields[0];

        QDateTime beginTime = QDateTime::fromString(newFields[1], "yyyy-M-d");
        if (!beginTime.isValid())
        {
            return;
        }
        stockData.m_beginTime = beginTime.toSecsSinceEpoch();

        QDateTime endTime = QDateTime::fromString(newFields[2], "yyyy-M-d");
        if (!endTime.isValid())
        {
            return;
        }
        stockData.m_endTime = endTime.toSecsSinceEpoch();

        for (int i=0; i<DATA_FIELD_LENGTH; i++)
        {
            stockData.m_data[i] = newFields[3+i];
        }
    }
    else
    {
        if (newFields.length() != 5)
        {
            return;
        }

        // 时数据，有大量不需要，优先过滤掉
        QString dateString;
        if (dataType == STOCK_DATA_HOUR)
        {
            stockData.m_hour = newFields[0].right(1);
            if (stockData.m_hour != YiWord && stockData.m_hour != WuWord && stockData.m_hour != WeiWord)
            {
                return;
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

    if (dataType == STOCK_DATA_HOUR)
    {
        // 时要分已午未
        if (stockData.m_hour == YiWord)
        {
            m_stockDatas[STOCK_DATA_HOUR_YI].append(stockData);
        }
        else if (stockData.m_hour == WuWord)
        {
            m_stockDatas[STOCK_DATA_HOUR_WU].append(stockData);
        }
        else if (stockData.m_hour == WeiWord)
        {
            m_stockDatas[STOCK_DATA_HOUR_WEI].append(stockData);
        }
    }
    else
    {
        m_stockDatas[dataType].append(stockData);
    }

    m_count++;
    if (m_count % 1000 == 0)
    {
        emit reportProgress(m_count);
        m_count = 0;
    }
}

LoadDataController::LoadDataController(QObject *parent)
    : QObject{parent}
{

}

void LoadDataController::run(QString rootDir)
{
    emit printLog(QString::fromWCharArray(L"开始加载数据"));

    emit printLog(QString::fromWCharArray(L"开始扫描数据文件"));
    m_stockFileScanner = new StockFileScanner();
    m_stockFileScanner->m_rootDir = rootDir;
    m_stockFileScanThread = new QThread();
    m_stockFileScanner->moveToThread(m_stockFileScanThread);
    connect(m_stockFileScanThread, &QThread::started, m_stockFileScanner, &StockFileScanner::run);
    connect(m_stockFileScanThread, &QThread::finished, m_stockFileScanThread, &QThread::deleteLater);
    connect(m_stockFileScanner, &StockFileScanner::runFinish, this, &LoadDataController::onStockFileScannerFinish);
    m_stockFileScanThread->start();
}

void LoadDataController::onStockFileScannerFinish()
{
    QVector<QString> stockFiles = m_stockFileScanner->m_stockFiles;
    QVector<QString> industryNames = m_stockFileScanner->m_industryNames;

    m_stockFileScanner->deleteLater();
    m_stockFileScanner = nullptr;
    m_stockFileScanThread->quit();
    m_stockFileScanThread = nullptr;

    emit printLog(QString::fromWCharArray(L"扫描数据文件完成"));

    if (stockFiles.size() == 0)
    {
        emit printLog(QString::fromWCharArray(L"没有找到数据文件"));
        emit runFinish();
        return;
    }

    emit printLog(QString::fromWCharArray(L"找到数据文件%1个").arg(stockFiles.size()));

    DataManager::getInstance()->clearData();

    // 开启多线程加载数据
    int threadCount = QThread::idealThreadCount();
    threadCount = qMin(threadCount, stockFiles.size());
    int fileCountPerThread = stockFiles.size() / threadCount;
    for (int i=0; i<threadCount; i++)
    {
        int begin = i * fileCountPerThread;
        int end = (i+1) * fileCountPerThread;
        if (i == threadCount-1)
        {
            end = stockFiles.size();
        }

        StockFileLoader* loader = new StockFileLoader();
        for (int j = begin; j < end; j++)
        {
            loader->m_stockFiles.append(stockFiles[j]);
            loader->m_industryNames.append(industryNames[j]);
        }

        QThread* thread = new QThread();
        loader->moveToThread(thread);
        connect(thread, &QThread::started, loader, &StockFileLoader::run);
        connect(thread, &QThread::finished, thread, &QThread::deleteLater);
        connect(loader, &StockFileLoader::reportProgress, this, &LoadDataController::onReportProgress);
        connect(loader, &StockFileLoader::runFinish, this, &LoadDataController::onStockFileLoaderFinish);
        m_stockFileLoaders.append(loader);
        m_loadDataThreads.append(thread);
    }

    emit printLog(QString::fromWCharArray(L"开始加载数据文件，使用线程数%1个").arg(threadCount));
    for (int i=0; i<m_loadDataThreads.size(); i++)
    {
        m_loadDataThreads[i]->start();
    }

    QTimer* timer = new QTimer(this);
    connect(timer, &QTimer::timeout, [this]() {
        emit printLog(QString::fromWCharArray(L"已加载数据%1条").arg(m_totalCount));
    });
    timer->start(5000);
}

void LoadDataController::onReportProgress(int count)
{
    m_totalCount += count;
}

void LoadDataController::onStockFileLoaderFinish(StockFileLoader* loader)
{
    for (int i=0; i<MAX_STOCK_DATA_COUNT; i++)
    {
        DataManager::getInstance()->m_stockDatas[i].append(loader->m_stockDatas[i]);
    }

    for (int i=0; i<m_stockFileLoaders.size(); i++)
    {
        if (m_stockFileLoaders[i] == loader)
        {
            loader->deleteLater();
            m_stockFileLoaders.remove(i);
            m_loadDataThreads[i]->quit();
            m_loadDataThreads.remove(i);
            break;
        }
    }

    if (m_stockFileLoaders.empty())
    {
        DataManager::getInstance()->sort();
        int totalCount = DataManager::getInstance()->totalCount();
        emit printLog(QString::fromWCharArray(L"加载数据完成，共%1条").arg(totalCount));
        emit runFinish();
    }
}
