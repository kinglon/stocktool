﻿#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QTextStream>
#include <QFile>
#include <QDateTime>
#include "datamanager.h"
#include "uiutil.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    setWindowFlags(windowFlags() & ~Qt::WindowMaximizeButtonHint);
    setWindowFlag(Qt::MSWindowsFixedSizeDialogHint, true);

    initCtrls();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::initCtrls()
{
    m_myChartWidget = new MyChartWidget(this);
    m_myChartWidget->setFixedWidth(ui->scrollArea->width());
    m_myChartWidget->setFixedHeight(0);
    ui->scrollArea->setWidget(m_myChartWidget);

    QDate now = QDate::currentDate();
    ui->beginDateEdit->setDate(now.addYears(-1));
    ui->endDateEdit->setDate(now);

    connect(ui->loadDataButton, &QPushButton::clicked, this, &MainWindow::onLoadDataButtonClicked);
    connect(ui->loadAssist1DataButton, &QPushButton::clicked, this, &MainWindow::onLoadAssist1DataButtonClicked);
    connect(ui->loadAssist2DataButton, &QPushButton::clicked, this, &MainWindow::onLoadAssist2DataButtonClicked);
    connect(ui->loadColorDataButton, &QPushButton::clicked, this, &MainWindow::onLoadColorDataButtonClicked);
    connect(ui->loadAssist1ColorDataButton, &QPushButton::clicked, this, &MainWindow::onLoadAssist1ColorDataButtonClicked);
    connect(ui->loadAssist2ColorDataButton, &QPushButton::clicked, this, &MainWindow::onLoadAssist2ColorDataButtonClicked);
    connect(ui->saveImageButton, &QPushButton::clicked, this, &MainWindow::onSaveImageButtonClicked);
    connect(ui->addAvgLineButton, &QPushButton::clicked, this, &MainWindow::onAddAvgLineButtonClicked);
}

void MainWindow::onLoadDataButtonClicked()
{
    bool ok = false;
    int maxCount = ui->maxCountEdit->text().toInt(&ok);
    if (!ok || maxCount <= 0)
    {
        UiUtil::showTip(QString::fromWCharArray(L"请填写正确的满柱数"));
        return;
    }

    QFileDialog fileDialog;
    fileDialog.setWindowTitle(QString::fromWCharArray(L"选择文件"));
    fileDialog.setNameFilters(QStringList() << "text files (*.txt)");
    if (fileDialog.exec() != QDialog::Accepted)
    {
        return;
    }

    QStringList selectedFiles = fileDialog.selectedFiles();
    QFile file(selectedFiles[0]);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        DataManager::getInstance()->m_avgLines.clear();
        DataManager::getInstance()->m_chartDatas.clear();
        QTextStream in(&file);
        in.setCodec("UTF-8");
        while (!in.atEnd())
        {
            QString line = in.readLine();
            ChartData chartData;
            if (!parseChartDataLine(line, chartData))
            {
                continue;
            }

            if (DataManager::getInstance()->m_chartDatas.empty())
            {
                DataManager::getInstance()->m_chartDatas.append(chartData);
                continue;
            }

            // 补充没有的数据项
            qint64 dayTime = DataManager::getInstance()->m_chartDatas[DataManager::getInstance()->m_chartDatas.length()-1].m_date;
            while (true)
            {
                QDateTime dateTime = QDateTime::fromSecsSinceEpoch(dayTime);
                if (chartData.m_type == CHART_DATA_TYPE_MONTH)
                {
                    dayTime = dateTime.addMonths(1).toSecsSinceEpoch();
                }
                else
                {
                    QDateTime nextDay = dateTime.addDays(1);
                    dayTime = nextDay.toSecsSinceEpoch();
                    if (nextDay.date().dayOfWeek() == 6 || nextDay.date().dayOfWeek() == 7)
                    {
                        continue;
                    }
                }

                if (dayTime >= chartData.m_date)
                {
                    break;
                }

                ChartData newChartData;
                newChartData.m_type = chartData.m_type;
                newChartData.m_count = 1;
                newChartData.m_date = dayTime;
                newChartData.m_color = RGB_ORANGE;
                DataManager::getInstance()->m_chartDatas.append(newChartData);
            }
            DataManager::getInstance()->m_chartDatas.append(chartData);
        }
        file.close();
    }

    if (m_myChartWidget)
    {
        m_myChartWidget->setMaxCount(maxCount);
        m_myChartWidget->onDataChanged();
    }
}

void MainWindow::onLoadAssist1DataButtonClicked()
{
    bool ok = false;
    int maxCount = ui->maxAssist1CountEdit->text().toInt(&ok);
    if (!ok || maxCount <= 0)
    {
        UiUtil::showTip(QString::fromWCharArray(L"请填写正确的满柱数"));
        return;
    }

    QFileDialog fileDialog;
    fileDialog.setWindowTitle(QString::fromWCharArray(L"选择文件"));
    fileDialog.setNameFilters(QStringList() << "text files (*.txt)");
    if (fileDialog.exec() != QDialog::Accepted)
    {
        return;
    }

    QStringList selectedFiles = fileDialog.selectedFiles();
    QFile file(selectedFiles[0]);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        DataManager::getInstance()->m_assist1Datas.clear();
        QTextStream in(&file);
        in.setCodec("UTF-8");
        while (!in.atEnd())
        {
            QString line = in.readLine();
            ChartData chartData;
            if (!parseChartDataLine(line, chartData))
            {
                continue;
            }
            DataManager::getInstance()->m_assist1Datas.append(chartData);
        }
        file.close();
    }

    if (m_myChartWidget)
    {
        m_myChartWidget->setMaxAssist1Count(maxCount);
        m_myChartWidget->onDataChanged();
    }
}

void MainWindow::onLoadAssist2DataButtonClicked()
{
    bool ok = false;
    int maxCount = ui->maxAssist2CountEdit->text().toInt(&ok);
    if (!ok || maxCount <= 0)
    {
        UiUtil::showTip(QString::fromWCharArray(L"请填写正确的满柱数"));
        return;
    }

    QFileDialog fileDialog;
    fileDialog.setWindowTitle(QString::fromWCharArray(L"选择文件"));
    fileDialog.setNameFilters(QStringList() << "text files (*.txt)");
    if (fileDialog.exec() != QDialog::Accepted)
    {
        return;
    }

    QStringList selectedFiles = fileDialog.selectedFiles();
    QFile file(selectedFiles[0]);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        DataManager::getInstance()->m_assist2Datas.clear();
        QTextStream in(&file);
        in.setCodec("UTF-8");
        while (!in.atEnd())
        {
            QString line = in.readLine();
            ChartData chartData;
            if (!parseChartDataLine(line, chartData))
            {
                continue;
            }
            DataManager::getInstance()->m_assist2Datas.append(chartData);
        }
        file.close();
    }

    if (m_myChartWidget)
    {
        m_myChartWidget->setMaxAssist2Count(maxCount);
        m_myChartWidget->onDataChanged();
    }
}

void MainWindow::onLoadColorDataButtonClicked()
{
    QString oneIncludeString = ui->oneIncludeEdit->text();
    QString twoIncludeString = ui->twoIncludeEdit->text();
    if (oneIncludeString.isEmpty() || twoIncludeString.isEmpty())
    {
        UiUtil::showTip(QString::fromWCharArray(L"请填写正确的一宫二宫含内容"));
        return;
    }

    QFileDialog fileDialog;
    fileDialog.setWindowTitle(QString::fromWCharArray(L"选择文件"));
    fileDialog.setNameFilters(QStringList() << "csv files (*.csv)");
    if (fileDialog.exec() != QDialog::Accepted)
    {
        return;
    }

    QStringList selectedFiles = fileDialog.selectedFiles();
    QFile file(selectedFiles[0]);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        DataManager::getInstance()->m_colorDatas.clear();
        QTextStream in(&file);
        while (!in.atEnd())
        {
            QString line = in.readLine();
            ColorData colorData;
            if (!parseColorDataLine(line, colorData, oneIncludeString, twoIncludeString))
            {
                continue;
            }
            DataManager::getInstance()->m_colorDatas.append(colorData);
        }
        file.close();
    }

    if (m_myChartWidget)
    {
        m_myChartWidget->onDataChanged();
    }
}

void MainWindow::onLoadAssist1ColorDataButtonClicked()
{
    QString oneIncludeString = ui->oneIncludeEdit->text();
    QString twoIncludeString = ui->twoIncludeEdit->text();
    if (oneIncludeString.isEmpty() || twoIncludeString.isEmpty())
    {
        UiUtil::showTip(QString::fromWCharArray(L"请填写正确的一宫二宫含内容"));
        return;
    }

    QFileDialog fileDialog;
    fileDialog.setWindowTitle(QString::fromWCharArray(L"选择文件"));
    fileDialog.setNameFilters(QStringList() << "csv files (*.csv)");
    if (fileDialog.exec() != QDialog::Accepted)
    {
        return;
    }

    QStringList selectedFiles = fileDialog.selectedFiles();
    QFile file(selectedFiles[0]);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        DataManager::getInstance()->m_assist1ColorDatas.clear();
        QTextStream in(&file);
        while (!in.atEnd())
        {
            QString line = in.readLine();
            ColorData colorData;
            if (!parseColorDataLine(line, colorData, oneIncludeString, twoIncludeString))
            {
                continue;
            }
            DataManager::getInstance()->m_assist1ColorDatas.append(colorData);
        }
        file.close();
    }

    if (m_myChartWidget)
    {
        m_myChartWidget->onDataChanged();
    }
}

void MainWindow::onLoadAssist2ColorDataButtonClicked()
{
    QString oneIncludeString = ui->oneIncludeEdit->text();
    QString twoIncludeString = ui->twoIncludeEdit->text();
    if (oneIncludeString.isEmpty() || twoIncludeString.isEmpty())
    {
        UiUtil::showTip(QString::fromWCharArray(L"请填写正确的一宫二宫含内容"));
        return;
    }

    QFileDialog fileDialog;
    fileDialog.setWindowTitle(QString::fromWCharArray(L"选择文件"));
    fileDialog.setNameFilters(QStringList() << "csv files (*.csv)");
    if (fileDialog.exec() != QDialog::Accepted)
    {
        return;
    }

    QStringList selectedFiles = fileDialog.selectedFiles();
    QFile file(selectedFiles[0]);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        DataManager::getInstance()->m_assist2ColorDatas.clear();
        QTextStream in(&file);
        while (!in.atEnd())
        {
            QString line = in.readLine();
            ColorData colorData;
            if (!parseColorDataLine(line, colorData, oneIncludeString, twoIncludeString))
            {
                continue;
            }
            DataManager::getInstance()->m_assist2ColorDatas.append(colorData);
        }
        file.close();
    }

    if (m_myChartWidget)
    {
        m_myChartWidget->onDataChanged();
    }
}

bool MainWindow::parseChartDataLine(const QString& line, ChartData& chartData)
{
    if (line.isEmpty())
    {
        return false;
    }

    QStringList fields = line.split(' ');
    if (fields.length() < 2)
    {
        return false;
    }

    chartData.m_count = fields.length()-1;
    chartData.m_color = RGB_RED;
    QString dateString = fields[0];
    QDateTime dateTime;
    if (dateString.indexOf(QString::fromWCharArray(L"日")) > 0)
    {
        chartData.m_type = CHART_DATA_TYPE_DAY;
        dateTime = QDateTime::fromString(dateString, QString::fromWCharArray(L"yyyy年M月d日"));
    }
    else
    {
        chartData.m_type = CHART_DATA_TYPE_MONTH;
        dateTime = QDateTime::fromString(dateString, QString::fromWCharArray(L"yyyy年M月"));
    }
    if (!dateTime.isValid())
    {
        return false;
    }
    chartData.m_date = dateTime.toSecsSinceEpoch();
    return true;
}

bool MainWindow::parseColorDataLine(const QString& line, ColorData& colorData, const QString& oneInclude, const QString& twoInclude)
{
    if (line.isEmpty())
    {
        return false;
    }

    QStringList fields = line.split(',');
    if (fields.length() < 5)
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

    const int dataLength = 4;
    QString datas[dataLength];
    bool dayType = newFields[0].indexOf(QString::fromWCharArray(L"日")) > 0;
    if (!dayType) // 月数据
    {
        if (newFields.length() != 7)
        {
            return false;
        }

        colorData.m_type = CHART_DATA_TYPE_MONTH;
        QDateTime dateTime = QDateTime::fromString(newFields[0], QString::fromWCharArray(L"yyyy年M月"));
        colorData.m_date = dateTime.toSecsSinceEpoch();
        for (int i=0; i<dataLength; i++)
        {
            datas[i] = newFields[3+i];
        }
    }
    else
    {
        if (newFields.length() != 5)
        {
            return false;
        }

        colorData.m_type = CHART_DATA_TYPE_DAY;
        QDateTime dateTime = QDateTime::fromString(newFields[0], QString::fromWCharArray(L"yyyy年 M月 d日"));
        colorData.m_date = dateTime.toSecsSinceEpoch();
        for (int i=0; i<dataLength; i++)
        {
            datas[i] = newFields[1+i];
        }
    }

    checkIfInclude(datas, oneInclude, twoInclude, colorData.oneInclude, colorData.twoInclude);

    return true;
}

void MainWindow::checkIfInclude(QString data[4], const QString& oneInclude, const QString& twoInclude, bool& isOneInclude, bool& isTwoInclude)
{
    QString data1 = data[0];
    QString data2 = data[1];

    // 有空字先借位
    if (data[0].indexOf(QString::fromWCharArray(L"空")) >= 0)
    {
        data[0] += data2;
    }
    if (data[1].indexOf(QString::fromWCharArray(L"空")) >= 0)
    {
        data[1] += data1;
    }

    // 删除：阳忌、昌科、曲科、阳（忌）、昌（科）、曲（科）
    for (int i=0; i<4; i++)
    {
        data[i].replace(QString::fromWCharArray(L"阳忌"), "");
        data[i].replace(QString::fromWCharArray(L"昌科"), "");
        data[i].replace(QString::fromWCharArray(L"曲科"), "");
        data[i].replace(QString::fromWCharArray(L"阳(忌)"), "");
        data[i].replace(QString::fromWCharArray(L"昌(科)"), "");
        data[i].replace(QString::fromWCharArray(L"曲(科)"), "");
    }

    // 一二宫有存字，就删除禄字
    if (data1.indexOf(QString::fromWCharArray(L"存")) >= 0 ||
            data2.indexOf(QString::fromWCharArray(L"存")) >= 0)
    {
        data[0].replace(QString::fromWCharArray(L"禄"), "");
        data[1].replace(QString::fromWCharArray(L"禄"), "");
    }

    // 一二宫有禄字，就删除存字
    if (data1.indexOf(QString::fromWCharArray(L"禄")) >= 0 ||
            data2.indexOf(QString::fromWCharArray(L"禄")) >= 0)
    {
        data[0].replace(QString::fromWCharArray(L"存"), "");
        data[1].replace(QString::fromWCharArray(L"存"), "");
    }

    // 检查一宫含，只要一个满足
    bool ok = false;
    for (int i=0; i<oneInclude.length(); i++)
    {
        QString word = oneInclude[i];
        if (word == QString::fromWCharArray(L"存"))
        {
            if (hasCunWord(data[0], data[0], data[1]))
            {
                ok = true;
                break;
            }
        }
        else
        {
            if (data[0].indexOf(word) >= 0 && haveWordWithoutKuohao(word, data))
            {
                ok = true;
                break;
            }
        }
    }
    isOneInclude = ok;

    // 检查二宫含，只要一个满足
    ok = false;
    for (int i=0; i<twoInclude.length(); i++)
    {
        QString word = twoInclude[i];
        if (word == QString::fromWCharArray(L"存"))
        {
            if (hasCunWord(data[1], data[0], data[1]))
            {
                ok = true;
                break;
            }
        }
        else
        {
            if (data[1].indexOf(word) >= 0 && haveWordWithoutKuohao(word, data))
            {
                ok = true;
                break;
            }
        }
    }
    isTwoInclude = ok;
}

bool MainWindow::hasCunWord(QString data, QString data1, QString data2)
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

bool MainWindow::haveWordWithoutKuohao(QString word, QString data[4])
{
    for (int j=0; j<4; j++)
    {
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

void MainWindow::onSaveImageButtonClicked()
{
    if (m_myChartWidget == nullptr)
    {
        return;
    }

    QPixmap pixmap(m_myChartWidget->size());
    m_myChartWidget->render(&pixmap);

    QString filePath = QFileDialog::getSaveFileName(this, "保存图片", "", "Images (*.png *.jpeg *.jpg)");
    if (!filePath.isEmpty())
    {
        pixmap.save(filePath);
    }
}

void MainWindow::onAddAvgLineButtonClicked()
{
    QVector<ChartData>& chartDatas = DataManager::getInstance()->m_chartDatas;
    if (chartDatas.empty())
    {
        UiUtil::showTip(QString::fromWCharArray(L"请先加载数据"));
        return;
    }

    QDate beginDate = ui->beginDateEdit->date();
    QDate endDate = ui->endDateEdit->date();
    if (beginDate > endDate)
    {
        UiUtil::showTip(QString::fromWCharArray(L"请正确填写日期范围"));
        return;
    }

    QDateTime beginDateTime;
    beginDateTime.setDate(beginDate);
    if (chartDatas[0].m_type == CHART_DATA_TYPE_MONTH)
    {
        beginDateTime.setDate(QDate(beginDate.year(), beginDate.month(), 1));
    }
    int beginTime = beginDateTime.toSecsSinceEpoch();

    QDateTime endDateTime;
    endDateTime.setDate(endDate);
    if (chartDatas[0].m_type == CHART_DATA_TYPE_MONTH)
    {
        endDateTime.setDate(QDate(endDate.year(), endDate.month(), 1));
    }
    int endTime = endDateTime.toSecsSinceEpoch();

    int begin = -1;
    int end = -1;
    int total = 0;
    for (int i=0; i<chartDatas.size(); i++)
    {
        if (chartDatas[i].m_date < beginTime)
        {
            continue;
        }

        if (chartDatas[i].m_date > endTime)
        {
            break;
        }

        if (begin == -1)
        {
            begin = i;
        }
        end = i;
        total += chartDatas[i].m_count;
    }

    if (begin >= 0 && end >= 0)
    {
        int count = total / (end - begin + 1);
        count = qMax(count, 1);

        AvgLine avgLine;
        avgLine.m_begin = begin;
        avgLine.m_end = end;
        avgLine.m_count = count;
        DataManager::getInstance()->m_avgLines.append(avgLine);
        if (m_myChartWidget)
        {
            m_myChartWidget->onDataChanged();
        }
    }
}
