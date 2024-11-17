#include "mergedatacontroller.h"
#include <QTimer>
#include <QFile>
#include <QDir>
#include <QDesktopServices>
#include <QUrl>
#include <QTextStream>
#include "Utility/ImPath.h"
#include "settingmanager.h"
#include "datamanager.h"
#include "loaddatacontroller.h"

DataMerger::DataMerger(QObject *parent)
    : QObject{parent}
{

}

void DataMerger::run()
{
    for (int i=0; i<m_stockPaths.size(); i++)
    {
        m_currentIndex = i;
        m_dayStockData.clear();
        m_monthStockData.clear();
        m_dayLineDatas.clear();

        doMerge();
        emit oneFinish();
    }
    emit runFinish(this);
}



void DataMerger::doMerge()
{
    // 加载数据
    if (!loadData())
    {
        return;
    }

    // 合并日线数据与日数据
    mergeDayAndDayLine();

    // 筛选符合条件的数据
    QVector<DayLineData> dayLineDatas;
    filterData(dayLineDatas);

    // 合并月数据
    mergeMonthData(dayLineDatas);

    // 保存
    save(dayLineDatas);
}

bool DataMerger::loadData()
{
    QDir dir(m_stockPaths[m_currentIndex]);
    QFileInfoList fileList = dir.entryInfoList(QDir::Files);
    QString dayFilePath, monthFilePath, dayLineFilePath;
    foreach (const QFileInfo &fileInfo, fileList)
    {
        QString fileName = fileInfo.fileName();
        QString filePath = m_stockPaths[m_currentIndex] + "\\" + fileName;
        if (fileName.indexOf(QString::fromWCharArray(L"日.csv")) > 0)
        {
            dayFilePath = filePath;
        }
        else if (fileName.indexOf(QString::fromWCharArray(L"月.csv")) > 0)
        {
            monthFilePath = filePath;
        }
        else if (fileName.indexOf(QString::fromWCharArray(L"日线")) > 0)
        {
            dayLineFilePath = filePath;
        }
    }
    if (dayFilePath.isEmpty() || monthFilePath.isEmpty() || dayLineFilePath.isEmpty())
    {
        return false;
    }

    QString stockName = dir.dirName();

    // 加载日数据
    QFile dayFile(dayFilePath);
    if (dayFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QTextStream in(&dayFile);
        while (!in.atEnd())
        {
            QString line = in.readLine();
            StockData stockData;
            if (StockFileLoader::parseOneLine(m_industryNames[m_currentIndex], stockName, STOCK_DATA_DAY, line, stockData))
            {
                m_dayStockData.append(stockData);
            }
        }
        dayFile.close();
    }

    // 加载月数据
    QFile monthFile(monthFilePath);
    if (monthFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QTextStream in(&monthFile);
        while (!in.atEnd())
        {
            QString line = in.readLine();
            StockData stockData;
            if (StockFileLoader::parseOneLine(m_industryNames[m_currentIndex], stockName, STOCK_DATA_MONTH, line, stockData))
            {
                m_monthStockData.append(stockData);
            }
        }
        monthFile.close();
    }

    // 加载日线数据
    QFile dayLineFile(dayLineFilePath);
    if (dayLineFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QTextStream in(&dayLineFile);
        while (!in.atEnd())
        {
            QString line = in.readLine();
            DayLineData dayLineData;
            if (parseDayLine(line, dayLineData))
            {
                m_dayLineDatas.append(dayLineData);
            }
        }
        dayLineFile.close();
    }

    return true;
}

bool DataMerger::parseDayLine(const QString& line, DayLineData& dayLineData)
{
    if (line.isEmpty())
    {
        return false;
    }

    QStringList fields = line.split(',');
    if (fields.length() < 7)
    {
        return false;
    }

    QStringList newFields;
    for (auto& field : fields)
    {
        newFields.append(field);
    }

    if (newFields[0] == QString::fromWCharArray(L"时间"))
    {
        return false;
    }

    QDateTime dateTime = QDateTime::fromString(newFields[0], "yyyy/M/d");
    if (!dateTime.isValid())
    {
        return false;
    }

    QDate date = dateTime.date();
    if (date.dayOfWeek() == 6 || date.dayOfWeek() == 7)
    {
        return false;
    }

    dayLineData.m_dayTime = dateTime.toSecsSinceEpoch();
    dayLineData.m_zhangfu = newFields[6];
    return true;
}

void DataMerger::mergeDayAndDayLine()
{
    int next = 0;
    for (auto& dayLineData : m_dayLineDatas)
    {
        for (int i=next; i<m_dayStockData.size(); i++)
        {
            if (m_dayStockData[i].m_beginTime == dayLineData.m_dayTime)
            {
                dayLineData.m_dayStockData = m_dayStockData[i];
                next = i+1;
                break;
            }
            else if (m_dayStockData[i].m_beginTime > dayLineData.m_dayTime)
            {
                break;
            }
            else
            {
                next = i+1;
            }
        }
    }
}

void DataMerger::filterData(QVector<DayLineData>& dayLineDatas)
{
    if (m_dayLineDatas.empty())
    {
        return;
    }

    qint64 begin = m_dayLineDatas[m_dayLineDatas.size()-1].m_dayTime + 24*3600;
    for (auto& dayStockData : m_dayStockData)
    {
        if (dayStockData.m_beginTime < begin)
        {
            continue;
        }

        for (int i=0; i<m_dayLineDatas.size(); i++)
        {
            bool ok = false;
            if (m_compare2Part)
            {
                if (dayStockData.m_data[0] == m_dayLineDatas[i].m_dayStockData.m_data[0]
                        && dayStockData.m_data[1] == m_dayLineDatas[i].m_dayStockData.m_data[1])
                {
                    ok = true;
                }
            }
            else
            {
                ok = true;
                for (int j=0; j<DATA_FIELD_LENGTH;j++)
                {
                    if (dayStockData.m_data[j] != m_dayLineDatas[i].m_dayStockData.m_data[j])
                    {
                        ok = false;
                        break;
                    }
                }
            }

            if (ok)
            {
                dayLineDatas.append(m_dayLineDatas[i]);
                m_dayLineDatas.remove(i);
                break;
            }
        }
    }
}

void DataMerger::mergeMonthData(QVector<DayLineData>& dayLineDatas)
{
    for (auto& dayLineData : dayLineDatas)
    {
        for (auto& monthData : m_monthStockData)
        {
            if (dayLineData.m_dayStockData.m_beginTime >= monthData.m_beginTime
                    && dayLineData.m_dayStockData.m_beginTime <= monthData.m_endTime)
            {
                dayLineData.m_monthStockData = monthData;
                break;
            }

            if (dayLineData.m_dayStockData.m_beginTime < monthData.m_beginTime)
            {
                break;
            }
        }
    }
}

void DataMerger::save(QVector<DayLineData>& dayLineDatas)
{
    if (dayLineDatas.empty())
    {
        return;
    }

    QString savePath = QString::fromStdWString(CImPath::GetDataPath()) + QString::fromWCharArray(L"合并\\");
    QDir dir;
    if (!dir.exists(savePath))
    {
        dir.mkpath(savePath);
    }

    if (!dayLineDatas[0].m_dayStockData.m_industryName.isEmpty())
    {
        savePath += dayLineDatas[0].m_dayStockData.m_industryName + "\\";
        if (!dir.exists(savePath))
        {
            dir.mkpath(savePath);
        }
    }

    // 按时间排序
    std::sort(dayLineDatas.begin(), dayLineDatas.end(), [](const DayLineData& a, const DayLineData& b) {
        return a.m_dayTime < b.m_dayTime;
    });

    QString result;
    for (auto& dayLineData : dayLineDatas)
    {
        result += QDateTime::fromSecsSinceEpoch(dayLineData.m_dayTime).toString("yyyy/M/d");
        result += "," + dayLineData.m_zhangfu;
        for (int i=0; i<DATA_FIELD_LENGTH; i++)
        {
            result += "," + dayLineData.m_dayStockData.m_data[i];
        }

        // 有月数据，显示月数据
        if (!dayLineData.m_monthStockData.m_stockName.isEmpty())
        {
            result += "," + dayLineData.m_monthStockData.m_lunarTime;
            for (int i=0; i<DATA_FIELD_LENGTH; i++)
            {
                result += "," + dayLineData.m_monthStockData.m_data[i];
            }
        }

        result += "\r\n";
    }

    QString resultFilePath = savePath + dayLineDatas[0].m_dayStockData.m_stockName + ".txt";
    QFile resultFile(resultFilePath);
    if (resultFile.open(QFile::WriteOnly))
    {
        resultFile.write(result.toUtf8());
        resultFile.close();
    }
    else
    {
        qCritical("failed to create the merge result file");
    }
}

MergeDataController::MergeDataController(QObject *parent)
    : QObject{parent}
{

}

void MergeDataController::run(QString rootDir, bool compare2Part)
{
    QString savePath = QString::fromStdWString(CImPath::GetDataPath()) + QString::fromWCharArray(L"合并\\");
    if (!removeDir(savePath))
    {
        emit printLog(QString::fromWCharArray(L"无法删除合并目录，请先关闭已打开的文件。"));
        emit runFinish();
        return;
    }

    emit printLog(QString::fromWCharArray(L"开始扫描股票"));
    scanStocks(rootDir);
    emit printLog(QString::fromWCharArray(L"扫描股票完成"));
    emit printLog(QString::fromWCharArray(L"股票数量：%1").arg(m_stockPaths.size()));
    if (m_stockPaths.isEmpty())
    {
        emit runFinish();
        return;
    }

    emit printLog(QString::fromWCharArray(L"开始合并数据"));

    // 开启多线程筛选数据
    int threadCount = QThread::idealThreadCount();
    threadCount = qMin(threadCount, m_stockPaths.size());
    int stockPerThread = m_stockPaths.size() / threadCount;
    for (int i=0; i<threadCount; i++)
    {
        int begin = i*stockPerThread;
        int end = (i+1)*stockPerThread;
        if (i == threadCount-1)
        {
            end = m_stockPaths.size();
        }

        DataMerger* dataMerger = new DataMerger();
        dataMerger->m_compare2Part = compare2Part;
        for (int j=begin; j<end; j++)
        {
            dataMerger->m_stockPaths.append(m_stockPaths[j]);
            dataMerger->m_industryNames.append(m_industryNames[j]);
        }

        QThread* thread = new QThread();
        dataMerger->moveToThread(thread);
        connect(thread, &QThread::started, dataMerger, &DataMerger::run);
        connect(thread, &QThread::finished, thread, &QThread::deleteLater);
        connect(dataMerger, &DataMerger::oneFinish, this, &MergeDataController::onMergeOneFinish);
        connect(dataMerger, &DataMerger::runFinish, this, &MergeDataController::onMergerRunFinish);
        m_dataMergers.append(dataMerger);
        m_dataMergerThreads.append(thread);
    }

    emit printLog(QString::fromWCharArray(L"开始合并数据，使用线程数%1个").arg(threadCount));
    for (int i=0; i<m_dataMergerThreads.size(); i++)
    {
        m_dataMergerThreads[i]->start();
    }

    QTimer* timer = new QTimer(this);
    connect(timer, &QTimer::timeout, [this]() {
        emit printLog(QString::fromWCharArray(L"合并进度：%1/%2").arg(QString::number(m_finishStocks), QString::number(m_stockPaths.size())));
    });
    timer->start(5000);
}

bool MergeDataController::removeDir(const QString &dirName)
{
    bool result = true;
    QDir dir(dirName);

    if (dir.exists())
    {
        // Get a list of all entries in the directory
        QFileInfoList entries = dir.entryInfoList(QDir::NoDotAndDotDot | QDir::System | QDir::Hidden  | QDir::AllDirs | QDir::Files, QDir::DirsFirst);

        // Iterate over the entries
        for (const QFileInfo &entry : entries)
        {
            if (entry.isDir())
            {
                // If the entry is a directory, recursively remove it
                result = removeDir(entry.absoluteFilePath());
            }
            else {
                // If it's a file, attempt to remove it
                result = QFile::remove(entry.absoluteFilePath());
            }

            if (!result)
            {
                return false; // Stop on first failure
            }
        }

        // After all contents are removed, attempt to remove the now-empty directory
        result = dir.rmdir(dirName);
    }

    return result;
}

void MergeDataController::onMergeOneFinish()
{
    m_finishStocks++;
}

void MergeDataController::onMergerRunFinish(DataMerger* dataMerger)
{    
    for (int i=0; i<m_dataMergers.size(); i++)
    {
        if (m_dataMergers[i] == dataMerger)
        {
            dataMerger->deleteLater();
            m_dataMergers.remove(i);
            m_dataMergerThreads[i]->quit();
            m_dataMergerThreads.remove(i);
            break;
        }
    }

    if (m_dataMergers.empty())
    {
        emit printLog(QString::fromWCharArray(L"合并数据完成"));
        QDesktopServices::openUrl(QUrl::fromLocalFile(QString::fromStdWString(CImPath::GetDataPath())));
        emit runFinish();
    }
}

void MergeDataController::scanStocks(QString rootDir)
{
    QDir dir(rootDir);
    QDir::Filters filters = QDir::Dirs | QDir::NoDotAndDotDot;
    QFileInfoList fileInfoList = dir.entryInfoList(filters);
    QString daySuffix = QString::fromWCharArray(L"日.csv");
    foreach (const QFileInfo &fileInfo, fileInfoList)
    {
        QString filePath = fileInfo.absoluteFilePath() + "\\" + fileInfo.fileName() + daySuffix;
        if (QFile(filePath).exists())
        {
            m_stockPaths.append(fileInfo.absoluteFilePath());
            m_industryNames.append("");
            continue;
        }

        QDir subDir(fileInfo.absoluteFilePath());
        QFileInfoList subFileInfoList = subDir.entryInfoList(filters);
        foreach (const QFileInfo &subFileInfo, subFileInfoList)
        {
            QString filePath = subFileInfo.absoluteFilePath() + "\\" + subFileInfo.fileName() + daySuffix;
            if (QFile(filePath).exists())
            {
                m_stockPaths.append(subFileInfo.absoluteFilePath());
                m_industryNames.append(fileInfo.fileName());
            }
        }
    }
}
