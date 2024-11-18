#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QTextStream>
#include <QFile>
#include <QDateTime>
#include "datamanager.h"

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

    connect(ui->loadDataButton, &QPushButton::clicked, this, &MainWindow::onLoadDataButtonClicked);
    connect(ui->saveImageButton, &QPushButton::clicked, this, &MainWindow::onSaveImageButtonClicked);
}

void MainWindow::onLoadDataButtonClicked()
{
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
