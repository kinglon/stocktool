#include "loaddatacontroller.h"
#include <QThread>
#include <QTimer>
#include <QDir>
#include <QTextStream>
#include <QDateTime>
#include "../Utility/ImPath.h"
#include "stockdatautil.h"
#include <QtMath>

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
    suffixes.append(QString::fromWCharArray(L"限.csv"));
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

        if (found)
        {
            // 添加日线文件
            QFileInfoList subFileInfoList = QDir(fileInfo.absoluteFilePath()).entryInfoList(QDir::Files);
            foreach (const QFileInfo &subFileInfo, subFileInfoList)
            {
                if (subFileInfo.fileName().indexOf(QString::fromWCharArray(L"日线")) >= 0)
                {
                    m_stockFiles.append(subFileInfo.absoluteFilePath());
                    m_industryNames.append("");
                    break;
                }
            }
        }
        else
        {
            // 行业下加载
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

                // 添加日线文件
                QFileInfoList subSubFileInfoList = QDir(subFileInfo.absoluteFilePath()).entryInfoList(QDir::Files);
                foreach (const QFileInfo &subSubFileInfo, subSubFileInfoList)
                {
                    if (subSubFileInfo.fileName().indexOf(QString::fromWCharArray(L"日线")) >= 0)
                    {
                        m_stockFiles.append(subSubFileInfo.absoluteFilePath());
                        m_industryNames.append(fileInfo.fileName());
                        break;
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
        if (fileName.indexOf(QString::fromWCharArray(L"日线")) >= 0)
        {
            // 日线数据另外加载
            loadDayLineData(industryName, stockFile);
        }
        else
        {
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

        m_count++;
        emit reportProgress(m_count);
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

    if (typeChar == QString::fromWCharArray(L"年"))
    {
        type = STOCK_DATA_YEAR;
        return true;
    }
    else if (typeChar == QString::fromWCharArray(L"月"))
    {
        type = STOCK_DATA_MONTH;
        return true;
    }
    else if (typeChar == QString::fromWCharArray(L"日"))
    {
        type = STOCK_DATA_DAY;
        return true;
    }
    else if (typeChar == QString::fromWCharArray(L"时"))
    {
        type = STOCK_DATA_HOUR;
        return true;
    }
    else if (typeChar == QString::fromWCharArray(L"限"))
    {
        type = STOCK_DATA_XIAN;
        return true;
    }
    else
    {
        qCritical("wrong file name: %s", fileName.toStdString().c_str());
        return false;
    }
}

void StockFileLoader::processOneLine(const QString& industryName, const QString& stockName, int dataType, const QString& line)
{
    StockData stockData;
    if (!StockDataUtil::parseOneLine(industryName, stockName, dataType, line, stockData))
    {
        return;
    }

    if (dataType == STOCK_DATA_HOUR)
    {
        // 时要分已午未
        if (stockData.m_hour == QString::fromWCharArray(L"巳"))
        {
            m_stockDatas[STOCK_DATA_HOUR_YI].append(stockData);
        }
        else if (stockData.m_hour == QString::fromWCharArray(L"午"))
        {
            m_stockDatas[STOCK_DATA_HOUR_WU].append(stockData);
        }
        else if (stockData.m_hour == QString::fromWCharArray(L"未"))
        {
            m_stockDatas[STOCK_DATA_HOUR_WEI].append(stockData);
        }
    }
    else
    {
        m_stockDatas[dataType].append(stockData);
    }    
}

void StockFileLoader::loadDayLineData(const QString& industryName, const QString& dayLineFilePath)
{
    // 获取股票名字
    QFileInfo fileInfo(dayLineFilePath);
    QStringList parts = fileInfo.fileName().split("_");
    if (parts.empty())
    {
        return;
    }
    QString stockName = parts[0];

    // 读取文件内容，解析数据
    QVector<DayLineData> dayLineDatas;
    QFile file(dayLineFilePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qCritical("failed to open file: %s", dayLineFilePath.toStdString().c_str());
        return;
    }
    QTextStream in(&file);
    while (!in.atEnd())
    {
        QString line = in.readLine();
        processOneLineOfDayLine(industryName, stockName, line, dayLineDatas);
    }
    file.close();

    if (dayLineDatas.empty())
    {
        return;
    }

    m_dayLineDatas[STOCK_DATA_DAY].append(dayLineDatas);

    // 统计年的数据
    calculateDayLineYear(dayLineDatas);

    // 统计月的数据
    calculateDayLineMonth(dayLineDatas);
}

void StockFileLoader::calculateDayLineYear(const QVector<DayLineData>& dayLineDatas)
{
    int currentYear = 0;
    int totalDay = 0;
    float startKaiPan = 0.0f;
    float totalHuanShou = 0.0f;
    for (int i=0; i<dayLineDatas.size(); i++)
    {
        const DayLineData& current = dayLineDatas[i];
        QDate currentDate = QDateTime::fromSecsSinceEpoch(current.m_beginTime).date();
        if (currentYear == 0)
        {
            // 年开始
            currentYear = currentDate.year();
            totalDay = 1;
            startKaiPan = current.m_kaiPan;
            totalHuanShou = current.m_huanShou;
            continue;
        }

        if (currentDate.year() == currentYear)
        {
            totalDay += 1;
            totalHuanShou += current.m_huanShou;
            continue;
        }
        else
        {
            // 新的一年
            DayLineData dayLineData;
            dayLineData.m_industryName = dayLineDatas[0].m_industryName;
            dayLineData.m_stockName = dayLineDatas[0].m_stockName;
            QDateTime beginDateTime;
            beginDateTime.setDate(QDate(currentYear, 1, 1));
            dayLineData.m_beginTime = beginDateTime.toSecsSinceEpoch();
            dayLineData.m_endTime = beginDateTime.addYears(1).toSecsSinceEpoch() - 1;
            dayLineData.m_totalDay = totalDay;
            if (qAbs(startKaiPan) >= 0.00001)
            {
                dayLineData.m_zhangFu = (dayLineDatas[i-1].m_kaiPan - startKaiPan) / startKaiPan *100;
            }
            dayLineData.m_huanShou = totalHuanShou;
            m_dayLineDatas[STOCK_DATA_YEAR].append(dayLineData);

            currentYear = 0;
            i--;
        }
    }

    // 最后一个
    DayLineData dayLineData;
    dayLineData.m_industryName = dayLineDatas[0].m_industryName;
    dayLineData.m_stockName = dayLineDatas[0].m_stockName;
    QDateTime beginDateTime;
    beginDateTime.setDate(QDate(currentYear, 1, 1));
    dayLineData.m_beginTime = beginDateTime.toSecsSinceEpoch();
    dayLineData.m_endTime = beginDateTime.addYears(1).toSecsSinceEpoch() - 1;
    dayLineData.m_totalDay = totalDay;
    if (qAbs(startKaiPan) >= 0.00001)
    {
        dayLineData.m_zhangFu = (dayLineDatas[dayLineDatas.size()-1].m_kaiPan - startKaiPan) / startKaiPan *100;
    }
    dayLineData.m_huanShou = totalHuanShou;
    m_dayLineDatas[STOCK_DATA_YEAR].append(dayLineData);
}

void StockFileLoader::calculateDayLineMonth(const QVector<DayLineData>& dayLineDatas)
{
    int currentMonth = -1;
    int currentYear = 0;
    int totalDay = 0;
    float startKaiPan = 0.0f;
    float totalHuanShou = 0.0f;
    for (int i=0; i<dayLineDatas.size(); i++)
    {
        const DayLineData& current = dayLineDatas[i];
        QDate currentDate = QDateTime::fromSecsSinceEpoch(current.m_beginTime).date();
        if (currentMonth == -1)
        {
            // 月开始
            currentMonth = currentDate.month();
            currentYear = currentDate.year();
            totalDay = 1;
            startKaiPan = current.m_kaiPan;
            totalHuanShou = current.m_huanShou;
            continue;
        }

        if (currentDate.month() == currentMonth)
        {
            totalDay += 1;
            totalHuanShou += current.m_huanShou;
            continue;
        }
        else
        {
            // 新的一月
            DayLineData dayLineData;
            dayLineData.m_industryName = dayLineDatas[0].m_industryName;
            dayLineData.m_stockName = dayLineDatas[0].m_stockName;
            QDateTime beginDateTime;
            beginDateTime.setDate(QDate(currentYear, currentMonth, 1));
            dayLineData.m_beginTime = beginDateTime.toSecsSinceEpoch();
            dayLineData.m_endTime = beginDateTime.addMonths(1).toSecsSinceEpoch() - 1;
            dayLineData.m_totalDay = totalDay;
            if (qAbs(startKaiPan) >= 0.00001)
            {
                dayLineData.m_zhangFu = (dayLineDatas[i-1].m_kaiPan - startKaiPan) / startKaiPan *100;
            }
            dayLineData.m_huanShou = totalHuanShou;
            m_dayLineDatas[STOCK_DATA_MONTH].append(dayLineData);

            currentMonth = -1;
            i--;
        }
    }

    // 最后一个
    DayLineData dayLineData;
    dayLineData.m_industryName = dayLineDatas[0].m_industryName;
    dayLineData.m_stockName = dayLineDatas[0].m_stockName;
    QDateTime beginDateTime;
    beginDateTime.setDate(QDate(currentYear, currentMonth, 1));
    dayLineData.m_beginTime = beginDateTime.toSecsSinceEpoch();
    dayLineData.m_endTime = beginDateTime.addMonths(1).toSecsSinceEpoch() - 1;
    dayLineData.m_totalDay = totalDay;
    if (qAbs(startKaiPan) >= 0.00001)
    {
        dayLineData.m_zhangFu = (dayLineDatas[dayLineDatas.size()-1].m_kaiPan - startKaiPan) / startKaiPan *100;
    }
    dayLineData.m_huanShou = totalHuanShou;
    m_dayLineDatas[STOCK_DATA_MONTH].append(dayLineData);
}

void StockFileLoader::processOneLineOfDayLine(const QString& industryName, const QString& stockName,
                                              const QString& line, QVector<DayLineData>& dayLineDatas)
{
    if (line.isEmpty())
    {
        return;
    }

    QStringList newFields = line.split(',');
    if (newFields.length() < 11) // 至少要有11个字段
    {
        return;
    }

    if (newFields[0] == QString::fromWCharArray(L"时间"))
    {
        return;
    }

    DayLineData dayLineData;
    dayLineData.m_industryName = industryName;
    dayLineData.m_stockName = stockName;
    QDateTime day = QDateTime::fromString(newFields[0], "yyyy/M/d");
    if (!day.isValid())
    {
        return;
    }
    dayLineData.m_beginTime = day.toSecsSinceEpoch();
    dayLineData.m_endTime = day.addDays(1).toSecsSinceEpoch() - 1;

    bool ok = false;
    float zhangFu = newFields[6].replace("%", "").toFloat(&ok);
    if (ok)
    {
        dayLineData.m_zhangFu = zhangFu;
    }

    float huanShou = newFields[10].replace("%", "").toFloat(&ok);
    if (ok)
    {
        dayLineData.m_huanShou = huanShou;
    }

    float kaiPan = newFields[2].replace("%", "").toFloat(&ok);
    if (ok)
    {
        dayLineData.m_kaiPan = kaiPan;
    }

    dayLineDatas.append(dayLineData);
}

LoadDataController::LoadDataController(QObject *parent)
    : QObject{parent}
{
    m_dataManager = DataManager::getInstance();
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

    m_dataManager->clearData();

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
    int totalFileCount = stockFiles.size();
    connect(timer, &QTimer::timeout, [this, totalFileCount]() {
        emit printLog(QString::fromWCharArray(L"已加载文件%1/%2").arg(
                          QString::number(m_totalCount), QString::number(totalFileCount)));
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
        m_dataManager->m_stockDatas[i].append(loader->m_stockDatas[i]);
        m_dataManager->m_dayLineDatas[i].append(loader->m_dayLineDatas[i]);
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
        m_dataManager->sort();
        int totalCount = m_dataManager->totalCount();
        emit printLog(QString::fromWCharArray(L"加载数据完成，共%1条").arg(totalCount));
        emit runFinish();
    }
}
