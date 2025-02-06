#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "../stock/stockdatautil.h"
#include "../stock/datamanager.h"
#include "../stock/loaddatacontroller.h"
#include "../Utility/uiutil.h"
#include "../Utility/ImPath.h"
#include "settingmanager.h"
#include <QFileDialog>
#include <QDesktopServices>
#include "scorecontroller.h"

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
    QDate now = QDate::currentDate();
    ui->beginDateEdit->setDate(now.addYears(-1));
    ui->endDateEdit->setDate(now);

    ui->dataTypeComboBox->addItem(QString::fromWCharArray(L"年"), STOCK_DATA_YEAR);
    ui->dataTypeComboBox->addItem(QString::fromWCharArray(L"月"), STOCK_DATA_MONTH);
    ui->dataTypeComboBox->addItem(QString::fromWCharArray(L"日"), STOCK_DATA_DAY);
    ui->dataTypeComboBox->setCurrentIndex(0);

    for (int i=0; i<SCORE_COUNT; i++)
    {
        QString scoreEditName = QString("lineEdit_%1").arg(i+1);
        QList<QLineEdit *> edits = findChildren<QLineEdit *>(scoreEditName);
        if (edits.size() > 0)
        {
            edits[0]->setText(QString::number(SettingManager::getInstance()->m_scores[i]));
        }
    }

    connect(ui->loadDataButton, &QPushButton::clicked, this, &MainWindow::onLoadDataButtonClicked);
    connect(ui->scoreButton, &QPushButton::clicked, this, &MainWindow::onScoreButtonClicked);
    connect(ui->exportButton, &QPushButton::clicked, this, &MainWindow::onExportButtonClicked);
}

void MainWindow::enableOperate(bool enable)
{
    ui->loadDataButton->setEnabled(enable);
    ui->scoreButton->setEnabled(enable);
    ui->exportButton->setEnabled(enable);
}

void MainWindow::onPrintLog(QString content)
{    
    QDateTime currentDateTime = QDateTime::currentDateTime();
    QString currentTimeString = currentDateTime.toString("[MM-dd hh:mm:ss] ");
    QString line = currentTimeString + content;
    ui->logTextEdit->append(line);
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

    m_loadDataPath = rootDir;

    ui->logTextEdit->clear();

    LoadDataController* controller = new LoadDataController(this);
    connect(controller, &LoadDataController::printLog, this, &MainWindow::onPrintLog);
    connect(controller, &LoadDataController::runFinish, [this, controller]() {
        controller->deleteLater();
        enableOperate(true);
    });
    enableOperate(false);
    controller->run(rootDir);
}

void MainWindow::onExportButtonClicked()
{
    if (m_scoreResult.isEmpty())
    {
        UiUtil::showTip(QString::fromWCharArray(L"请先打分"));
        return;
    }

    QString now = QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss");
    QString resultFilePath = QString::fromStdWString(CImPath::GetDataPath()) + QDir(m_loadDataPath).dirName() + QString::fromWCharArray(L"_得分_") + now + QString::fromWCharArray(L".txt");
    QFile resultFile(resultFilePath);
    if (resultFile.open(QFile::WriteOnly))
    {
        resultFile.write(m_scoreResult.toUtf8());
        resultFile.close();
        QDesktopServices::openUrl(QUrl::fromLocalFile(QString::fromStdWString(CImPath::GetDataPath())));
    }
}

void MainWindow::onScoreButtonClicked()
{
    if (!DataManager::getInstance()->hasData())
    {
        UiUtil::showTip(QString::fromWCharArray(L"请先加载数据"));
        return;
    }

    // 保存分数设置条件
    for (int i=0; i<SCORE_COUNT; i++)
    {
        QString scoreEditName = QString("lineEdit_%1").arg(i+1);
        QList<QLineEdit *> edits = findChildren<QLineEdit *>(scoreEditName);
        if (edits.empty())
        {
            return;
        }

        bool ok = false;
        int value = edits[0]->text().toInt(&ok);
        if (!ok || value < 0)
        {
            UiUtil::showTip(QString::fromWCharArray(L"请正确设置分数"));
            return;
        }

        SettingManager::getInstance()->m_scores[i] = value;
    }

    SettingManager::getInstance()->save();

    // 获取打分日期范围
    QDate beginDate = ui->beginDateEdit->date();
    QDate endDate = ui->endDateEdit->date().addDays(1); // 打分的时候不含endDate这一天
    if (beginDate >= endDate)
    {
        UiUtil::showTip(QString::fromWCharArray(L"请正确设置打分时间段"));
        return;
    }

    ScoreParam scoreParam;
    scoreParam.m_beginDate = beginDate;
    scoreParam.m_endDate = endDate;
    scoreParam.m_dataType = ui->dataTypeComboBox->currentData().toInt();
    scoreParam.m_matchAll = ui->matchAllCheckBox->isChecked();
    scoreParam.m_isGetMax = ui->getMaxButton->isChecked();
    if (!scoreParam.m_isGetMax)
    {
        bool ok = false;
        scoreParam.m_sumFactor = ui->sumFactorEdit->text().toInt(&ok);
        if (!ok || scoreParam.m_sumFactor <= 0)
        {
            UiUtil::showTip(QString::fromWCharArray(L"请正确填写相加系数"));
            return;
        }
    }
    scoreParam.m_orderUp = ui->orderCheckBox->isChecked();
    scoreParam.m_name = QDir(m_loadDataPath).dirName();

    ui->logTextEdit->clear();

    ScoreDataController* controller = new ScoreDataController(this);
    connect(controller, &ScoreDataController::printLog, this, &MainWindow::onPrintLog);
    connect(controller, &ScoreDataController::runFinish, [this, controller]() {
        m_scoreResult = controller->m_resultString;
        if (!m_scoreResult.isEmpty())
        {
            ui->logTextEdit->setText(controller->m_resultString);
        }
        controller->deleteLater();
        enableOperate(true);
    });
    enableOperate(false);
    controller->run(scoreParam);
}
