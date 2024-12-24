#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "uiutil.h"
#include <QDateTime>
#include <QFileDialog>
#include "Utility/LogUtil.h"
#include "loaddatacontroller.h"
#include "filterdatacontroller.h"
#include "daymergedatacontroller.h"
#include "monthmergedatacontroller.h"
#include "datamanager.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    setWindowFlags(windowFlags() & ~Qt::WindowMaximizeButtonHint);
    setWindowFlag(Qt::MSWindowsFixedSizeDialogHint, true);

    initCtrls();

    m_ctrlDShortcut = new QShortcut(QKeySequence("Ctrl+D"), this);
    connect(m_ctrlDShortcut, &QShortcut::activated, this, &MainWindow::onCtrlDShortcut);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::initCtrls()
{
    // 初始过滤条件
    for (int i=0; i<MAX_FILTER_CONDTION_COUNT; i++)
    {
        QString editCtrlName = QString("lineEdit_%1").arg(i*4+1);
        findChild<QLineEdit*>(editCtrlName)->setText(SettingManager::getInstance()->m_filterCondition[i].m_oneInclude);
        editCtrlName = QString("lineEdit_%1").arg(i*4+2);
        findChild<QLineEdit*>(editCtrlName)->setText(SettingManager::getInstance()->m_filterCondition[i].m_oneExclude);
        editCtrlName = QString("lineEdit_%1").arg(i*4+3);
        findChild<QLineEdit*>(editCtrlName)->setText(SettingManager::getInstance()->m_filterCondition[i].m_twoInclude);
        editCtrlName = QString("lineEdit_%1").arg(i*4+4);
        findChild<QLineEdit*>(editCtrlName)->setText(SettingManager::getInstance()->m_filterCondition[i].m_twoExclude);
    }

    ui->beginDateEdit->setDate(QDate::currentDate().addYears(-1));
    ui->endDateEdit->setDate(QDate::currentDate());

    connect(ui->loadDataButton, &QPushButton::clicked, this, &MainWindow::onLoadDataButtonClicked);
    connect(ui->filterDataButton, &QPushButton::clicked, this, &MainWindow::onFilterDataButtonClicked);
    connect(ui->filterHourDataButton, &QPushButton::clicked, this, &MainWindow::onFilterHourDataButtonClicked);
    connect(ui->dayCompareButton, &QPushButton::clicked, this, &MainWindow::onDayCompareButtonClicked);
    connect(ui->monthCompareButton, &QPushButton::clicked, this, &MainWindow::onMonthCompareButtonClicked);
}

void MainWindow::enableOperate(bool enable)
{
    ui->loadDataButton->setEnabled(enable);
    ui->filterDataButton->setEnabled(enable);
    ui->filterHourDataButton->setEnabled(enable);
    ui->dayCompareButton->setEnabled(enable);
    ui->monthCompareButton->setEnabled(enable);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    event->accept();
}

void MainWindow::onCtrlDShortcut()
{

}

void MainWindow::onPrintLog(QString content)
{
    static int lineCount = 0;
    if (lineCount >= 1000)
    {
        ui->logEdit->clear();
        lineCount = 0;
    }
    lineCount++;

    qInfo(content.toStdString().c_str());
    QDateTime currentDateTime = QDateTime::currentDateTime();
    QString currentTimeString = currentDateTime.toString("[MM-dd hh:mm:ss] ");
    QString line = currentTimeString + content;
    ui->logEdit->append(line);
}

void MainWindow::onLoadDataButtonClicked()
{
    if (DataManager::getInstance()->hasData())
    {
        if (!UiUtil::showTipV2(QString::fromWCharArray(L"确定重新加载数据?")))
        {
            return;
        }
    }

    QString rootDir = QFileDialog::getExistingDirectory(this, QString::fromWCharArray(L"选择股票数据根目录"),
                                                      QDir::homePath());
    if (rootDir.isEmpty())
    {
        return;
    }

    LoadDataController* controller = new LoadDataController(this);
    connect(controller, &LoadDataController::printLog, this, &MainWindow::onPrintLog);
    connect(controller, &LoadDataController::runFinish, [this, controller]() {
        controller->deleteLater();
        enableOperate(true);
    });
    enableOperate(false);
    controller->run(rootDir);
}

void MainWindow::onFilterDataButtonClicked()
{
    doFilter(false);
}

void MainWindow::onFilterHourDataButtonClicked()
{
    doFilter(true);
}


void MainWindow::doFilter(bool filterHour)
{
    if (!DataManager::getInstance()->hasData())
    {
        UiUtil::showTip(QString::fromWCharArray(L"请先加载数据"));
        return;
    }

    // 保存过滤条件    
    for (int i=0; i<MAX_FILTER_CONDTION_COUNT; i++)
    {
        QString editCtrlName = QString("lineEdit_%1").arg(i*4+1);
        SettingManager::getInstance()->m_filterCondition[i].m_oneInclude = findChild<QLineEdit*>(editCtrlName)->text();
        editCtrlName = QString("lineEdit_%1").arg(i*4+2);
        SettingManager::getInstance()->m_filterCondition[i].m_oneExclude = findChild<QLineEdit*>(editCtrlName)->text();
        editCtrlName = QString("lineEdit_%1").arg(i*4+3);
        SettingManager::getInstance()->m_filterCondition[i].m_twoInclude = findChild<QLineEdit*>(editCtrlName)->text();
        editCtrlName = QString("lineEdit_%1").arg(i*4+4);
        SettingManager::getInstance()->m_filterCondition[i].m_twoExclude = findChild<QLineEdit*>(editCtrlName)->text();        
    }    
    SettingManager::getInstance()->save();

    // 获取筛选时间
    bool onlyFilterToMonth = ui->onlyFilterMonthCheckBox->isChecked();
    QDate beginDate = ui->beginDateEdit->date();
    QDate endDate = ui->endDateEdit->date().addDays(1); // 筛选的时候不含endDate这一天
    if (beginDate > endDate)
    {
        UiUtil::showTip(QString::fromWCharArray(L"请正确设置筛选时间"));
        return;
    }

    FilterDataController* controller = new FilterDataController(this);
    connect(controller, &FilterDataController::printLog, this, &MainWindow::onPrintLog);
    connect(controller, &FilterDataController::runFinish, [this, controller]() {
        controller->deleteLater();
        enableOperate(true);
    });
    enableOperate(false);
    controller->run(filterHour, onlyFilterToMonth, beginDate, endDate);
}

void MainWindow::onDayCompareButtonClicked()
{
    qint64 beginDate = ui->beginDateEdit->dateTime().toSecsSinceEpoch();
    qint64 endDate = ui->endDateEdit->dateTime().addDays(1).toSecsSinceEpoch(); // 合并的时候不含endDate这一天
    if (beginDate > endDate)
    {
        UiUtil::showTip(QString::fromWCharArray(L"请正确设置合并时间"));
        return;
    }

    QString rootDir = QFileDialog::getExistingDirectory(this, QString::fromWCharArray(L"选择合并数据根目录"),
                                                      QDir::homePath());
    if (rootDir.isEmpty())
    {
        return;
    }

    bool compare2Part = ui->compare2PartButton->isChecked();
    DayMergeDataController* controller = new DayMergeDataController(this);
    controller->m_beginDate = beginDate;
    controller->m_endDate = endDate;
    connect(controller, &MergeDataController::printLog, this, &MainWindow::onPrintLog);
    connect(controller, &MergeDataController::runFinish, [this, controller]() {
        controller->deleteLater();
        enableOperate(true);
    });
    enableOperate(false);
    controller->run(rootDir, compare2Part);
}

void MainWindow::onMonthCompareButtonClicked()
{
    QString rootDir = QFileDialog::getExistingDirectory(this, QString::fromWCharArray(L"选择合并数据根目录"),
                                                      QDir::homePath());
    if (rootDir.isEmpty())
    {
        return;
    }

    bool compare2Part = ui->compare2PartButton->isChecked();
    MonthMergeDataController* controller = new MonthMergeDataController(this);
    connect(controller, &MergeDataController::printLog, this, &MainWindow::onPrintLog);
    connect(controller, &MergeDataController::runFinish, [this, controller]() {
        controller->deleteLater();
        enableOperate(true);
    });
    enableOperate(false);
    controller->run(rootDir, compare2Part);
}
