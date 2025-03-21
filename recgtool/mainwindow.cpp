﻿#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "settingmanager.h"
#include <QDate>
#include "../Utility/uiutil.h"
#include "stockdatamanager.h"
#include <QFileDialog>
#include <QFile>
#include "maincontroller.h"
#include "stockdatawriter.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    setWindowFlags(windowFlags() & ~Qt::WindowMaximizeButtonHint);
    setWindowFlag(Qt::MSWindowsFixedSizeDialogHint, true);

    initCtrls();
    initActions();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::initCtrls()
{
    QString stockPath = SettingManager::getInstance()->m_stockSetting[STOCK_TYPE_ZHISHU].m_stockPath;
    ui->lineEditZhiShuGuPiao->setText(stockPath);
    ui->checkBoxZhiShuZhangDie->setChecked(SettingManager::getInstance()->m_stockSetting[STOCK_TYPE_ZHISHU].m_enableZhangDie);
    ui->lineEditZhiShuZhangDieStart->setText(QString::number(SettingManager::getInstance()->m_stockSetting[STOCK_TYPE_ZHISHU].m_zhangDieStart, 'f', 2));
    ui->lineEditZhiShuZhangDieEnd->setText(QString::number(SettingManager::getInstance()->m_stockSetting[STOCK_TYPE_ZHISHU].m_zhangDieEnd, 'f', 2));
    ui->lineEditZhiShuPercent->setText(QString::number(SettingManager::getInstance()->m_stockSetting[STOCK_TYPE_ZHISHU].m_kuoJianPercent*100, 'f', 2));

    stockPath = SettingManager::getInstance()->m_stockSetting[STOCK_TYPE_HANGYE].m_stockPath;
    ui->lineEditHangYeGuPiao->setText(stockPath);
    ui->checkBoxHangYeZhangDie->setChecked(SettingManager::getInstance()->m_stockSetting[STOCK_TYPE_HANGYE].m_enableZhangDie);
    ui->lineEditHangYeZhangDieStart->setText(QString::number(SettingManager::getInstance()->m_stockSetting[STOCK_TYPE_HANGYE].m_zhangDieStart, 'f', 2));
    ui->lineEditHangYeZhangDieEnd->setText(QString::number(SettingManager::getInstance()->m_stockSetting[STOCK_TYPE_HANGYE].m_zhangDieEnd, 'f', 2));
    ui->lineEditHangyePercent->setText(QString::number(SettingManager::getInstance()->m_stockSetting[STOCK_TYPE_HANGYE].m_kuoJianPercent*100, 'f', 2));

    stockPath = SettingManager::getInstance()->m_stockSetting[STOCK_TYPE_GAINIAN1].m_stockPath;
    ui->lineEditGaiNian1GuPiao->setText(stockPath);
    ui->checkBoxGaiNian1ZhangDie->setChecked(SettingManager::getInstance()->m_stockSetting[STOCK_TYPE_GAINIAN1].m_enableZhangDie);
    ui->lineEditGaiNian1ZhangDieStart->setText(QString::number(SettingManager::getInstance()->m_stockSetting[STOCK_TYPE_GAINIAN1].m_zhangDieStart, 'f', 2));
    ui->lineEditGaiNian1ZhangDieEnd->setText(QString::number(SettingManager::getInstance()->m_stockSetting[STOCK_TYPE_GAINIAN1].m_zhangDieEnd, 'f', 2));
    ui->lineEditGaiNian1Percent->setText(QString::number(SettingManager::getInstance()->m_stockSetting[STOCK_TYPE_GAINIAN1].m_kuoJianPercent*100, 'f', 2));

    stockPath = SettingManager::getInstance()->m_stockSetting[STOCK_TYPE_GAINIAN2].m_stockPath;
    ui->lineEditGaiNian2GuPiao->setText(stockPath);
    ui->checkBoxGaiNian2ZhangDie->setChecked(SettingManager::getInstance()->m_stockSetting[STOCK_TYPE_GAINIAN2].m_enableZhangDie);
    ui->lineEditGaiNian2ZhangDieStart->setText(QString::number(SettingManager::getInstance()->m_stockSetting[STOCK_TYPE_GAINIAN2].m_zhangDieStart, 'f', 2));
    ui->lineEditGaiNian2ZhangDieEnd->setText(QString::number(SettingManager::getInstance()->m_stockSetting[STOCK_TYPE_GAINIAN2].m_zhangDieEnd, 'f', 2));
    ui->lineEditGaiNian2Percent->setText(QString::number(SettingManager::getInstance()->m_stockSetting[STOCK_TYPE_GAINIAN2].m_kuoJianPercent*100, 'f', 2));

    stockPath = SettingManager::getInstance()->m_stockSetting[STOCK_TYPE_GEGU].m_stockPath;
    ui->lineEditGeGuGuPiao->setText(stockPath);
    ui->checkBoxGeGuZhangDie->setChecked(SettingManager::getInstance()->m_stockSetting[STOCK_TYPE_GEGU].m_enableZhangDie);
    ui->lineEditGeGuZhangDieStart->setText(QString::number(SettingManager::getInstance()->m_stockSetting[STOCK_TYPE_GEGU].m_zhangDieStart, 'f', 2));
    ui->lineEditGeGuZhangDieEnd->setText(QString::number(SettingManager::getInstance()->m_stockSetting[STOCK_TYPE_GEGU].m_zhangDieEnd, 'f', 2));

    ui->dateEditZhiding->setDate(QDate::currentDate());

    if (SettingManager::getInstance()->m_recgDateStart > 0)
    {
        ui->dateEditRecgStart->setDate(QDateTime::fromSecsSinceEpoch(SettingManager::getInstance()->m_recgDateStart).date());
    }
    else
    {
        QDate beginDate(2022, 3, 1);
        ui->dateEditRecgStart->setDate(beginDate);
    }

    if (SettingManager::getInstance()->m_recgDateEnd > 0)
    {
        ui->dateEditRecgEnd->setDate(QDateTime::fromSecsSinceEpoch(SettingManager::getInstance()->m_recgDateEnd).date());
    }
    else
    {
        ui->dateEditRecgEnd->setDate(QDate::currentDate());
    }
}

void MainWindow::initActions()
{
    connect(ui->pushButtonStart, &QPushButton::clicked, this, &MainWindow::onStartButtonClicked);
    connect(ui->pushButtonSave, &QPushButton::clicked, this, &MainWindow::onSaveButtonClicked);

    connect(ui->pushButtonZhiShuGuPiao, &QPushButton::clicked, [this]() {
        QString stockPath = QFileDialog::getExistingDirectory(this, QString::fromWCharArray(L"选择目录"),
                                                              QDir::homePath());
        if (!stockPath.isEmpty())
        {
            ui->lineEditZhiShuGuPiao->setText(stockPath);
        }
    });

    connect(ui->pushButtonHangYeGuPiao, &QPushButton::clicked, [this]() {
        QString stockPath = QFileDialog::getExistingDirectory(this, QString::fromWCharArray(L"选择目录"),
                                                              QDir::homePath());
        if (!stockPath.isEmpty())
        {
            ui->lineEditHangYeGuPiao->setText(stockPath);
        }
    });

    connect(ui->pushButtonGaiNian1GuPiao, &QPushButton::clicked, [this]() {
        QString stockPath = QFileDialog::getExistingDirectory(this, QString::fromWCharArray(L"选择目录"),
                                                              QDir::homePath());
        if (!stockPath.isEmpty())
        {
            ui->lineEditGaiNian1GuPiao->setText(stockPath);
        }
    });

    connect(ui->pushButtonGaiNian2GuPiao, &QPushButton::clicked, [this]() {
        QString stockPath = QFileDialog::getExistingDirectory(this, QString::fromWCharArray(L"选择目录"),
                                                              QDir::homePath());
        if (!stockPath.isEmpty())
        {
            ui->lineEditGaiNian2GuPiao->setText(stockPath);
        }
    });

    connect(ui->pushButtonGeGuGuPiao, &QPushButton::clicked, [this]() {
        QString stockPath = QFileDialog::getExistingDirectory(this, QString::fromWCharArray(L"选择目录"),
                                                              QDir::homePath());
        if (!stockPath.isEmpty())
        {
            ui->lineEditGeGuGuPiao->setText(stockPath);
        }
    });
}

void MainWindow::onPrintLog(QString content)
{
    static int lineCount = 0;
    if (lineCount >= 1000)
    {
        ui->textEditLog->clear();
        lineCount = 0;
    }
    lineCount++;

    qInfo(content.toStdString().c_str());
    QDateTime currentDateTime = QDateTime::currentDateTime();
    QString currentTimeString = currentDateTime.toString("[MM-dd hh:mm:ss] ");
    QString line = currentTimeString + content;
    ui->textEditLog->append(line);
}

void MainWindow::onPrintResult(QString result)
{
    ui->textEditLog->setText(result);
}

void MainWindow::onSaveButtonClicked()
{
    if (!StockDataManager::getInstance()->hasResultData())
    {
        UiUtil::showTip(QString::fromWCharArray(L"请先识别"));
        return;
    }

    QString defaultDir = QDir::homePath();
    if (!SettingManager::getInstance()->m_savedPath.isEmpty())
    {
        defaultDir = SettingManager::getInstance()->m_savedPath;
    }
    QString saveDir = QFileDialog::getExistingDirectory(this, QString::fromWCharArray(L"选择存储目录"),
                                                          defaultDir);
    if (saveDir.isEmpty())
    {
        return;
    }

    SettingManager::getInstance()->m_savedPath = saveDir;
    SettingManager::getInstance()->save();

    if (!StockDataWriter::saveResult(saveDir))
    {
        UiUtil::showTip(QString::fromWCharArray(L"存储失败"));
        return;
    }

    UiUtil::showTip(QString::fromWCharArray(L"存储成功"));
}

void MainWindow::onStartButtonClicked()
{
    SettingManager* setting = SettingManager::getInstance();

    // 指数相关部分
    StockSetting* stockSetting = &setting->m_stockSetting[STOCK_TYPE_ZHISHU];
    stockSetting->m_enable = ui->checkBoxZhiShuGuPiao->isChecked();
    stockSetting->m_stockPath = ui->lineEditZhiShuGuPiao->text();
    if (stockSetting->m_enable)
    {
        if (!QFile(stockSetting->m_stockPath).exists())
        {
            UiUtil::showTip(QString::fromWCharArray(L"请选择指数股票目录"));
            return;
        }
    }

    stockSetting->m_enableZhangDie = ui->checkBoxZhiShuZhangDie->isChecked();

    bool ok1 = false;
    stockSetting->m_zhangDieStart = ui->lineEditZhiShuZhangDieStart->text().toFloat(&ok1);
    bool ok2 = false;
    stockSetting->m_zhangDieEnd = ui->lineEditZhiShuZhangDieEnd->text().toFloat(&ok2);
    if (!ok1 || !ok2 || stockSetting->m_zhangDieStart > stockSetting->m_zhangDieEnd)
    {
        UiUtil::showTip(QString::fromWCharArray(L"请正确填写指数涨跌幅"));
        return;
    }

    bool ok = false;
    stockSetting->m_kuoJianPercent = ui->lineEditZhiShuPercent->text().toFloat(&ok);
    if (!ok)
    {
        UiUtil::showTip(QString::fromWCharArray(L"请正确填写值算法中的指数部分"));
        return;
    }
    stockSetting->m_kuoJianPercent /= 100;

    // 行业相关部分
    stockSetting = &setting->m_stockSetting[STOCK_TYPE_HANGYE];
    stockSetting->m_enable = ui->checkBoxHangyeGuPiao->isChecked();
    stockSetting->m_stockPath = ui->lineEditHangYeGuPiao->text();
    if (stockSetting->m_enable)
    {        
        if (!QFile(stockSetting->m_stockPath).exists())
        {
            UiUtil::showTip(QString::fromWCharArray(L"请选择行业股票目录"));
            return;
        }
    }

    stockSetting->m_enableZhangDie = ui->checkBoxHangYeZhangDie->isChecked();
    ok1 = false;
    stockSetting->m_zhangDieStart = ui->lineEditHangYeZhangDieStart->text().toFloat(&ok1);
    ok2 = false;
    stockSetting->m_zhangDieEnd = ui->lineEditHangYeZhangDieEnd->text().toFloat(&ok2);
    if (!ok1 || !ok2 || stockSetting->m_zhangDieStart > stockSetting->m_zhangDieEnd)
    {
        UiUtil::showTip(QString::fromWCharArray(L"请正确填写行业涨跌幅"));
        return;
    }

    ok = false;
    stockSetting->m_kuoJianPercent = ui->lineEditHangyePercent->text().toFloat(&ok);
    if (!ok)
    {
        UiUtil::showTip(QString::fromWCharArray(L"请正确填写值算法中的行业部分"));
        return;
    }
    stockSetting->m_kuoJianPercent /= 100;

    // 概念1相关部分
    stockSetting = &setting->m_stockSetting[STOCK_TYPE_GAINIAN1];
    stockSetting->m_enable = ui->checkBoxGaiNian1GuPiao->isChecked();
    stockSetting->m_stockPath = ui->lineEditGaiNian1GuPiao->text();
    if (stockSetting->m_enable)
    {        
        if (!QFile(stockSetting->m_stockPath).exists())
        {
            UiUtil::showTip(QString::fromWCharArray(L"请选择概念1股票目录"));
            return;
        }
    }

    stockSetting->m_enableZhangDie = ui->checkBoxGaiNian1ZhangDie->isChecked();
    ok1 = false;
    stockSetting->m_zhangDieStart = ui->lineEditGaiNian1ZhangDieStart->text().toFloat(&ok1);

    ok2 = false;
    stockSetting->m_zhangDieEnd = ui->lineEditGaiNian1ZhangDieEnd->text().toFloat(&ok2);

    if (!ok1 || !ok2 || stockSetting->m_zhangDieStart > stockSetting->m_zhangDieEnd)
    {
        UiUtil::showTip(QString::fromWCharArray(L"请正确填写概念1涨跌幅"));
        return;
    }

    ok = false;
    stockSetting->m_kuoJianPercent = ui->lineEditGaiNian1Percent->text().toFloat(&ok);
    if (!ok)
    {
        UiUtil::showTip(QString::fromWCharArray(L"请正确填写值算法中的概念1部分"));
        return;
    }
    stockSetting->m_kuoJianPercent /= 100;

    // 概念2相关部分
    stockSetting = &setting->m_stockSetting[STOCK_TYPE_GAINIAN2];
    stockSetting->m_enable = ui->checkBoxGaiNian2GuPiao->isChecked();
    stockSetting->m_stockPath = ui->lineEditGaiNian2GuPiao->text();
    if (stockSetting->m_enable)
    {        
        if (!QFile(stockSetting->m_stockPath).exists())
        {
            UiUtil::showTip(QString::fromWCharArray(L"请选择概念2股票目录"));
            return;
        }
    }

    stockSetting->m_enableZhangDie = ui->checkBoxGaiNian2ZhangDie->isChecked();
    ok1 = false;
    stockSetting->m_zhangDieStart = ui->lineEditGaiNian2ZhangDieStart->text().toFloat(&ok1);

    ok2 = false;
    stockSetting->m_zhangDieEnd = ui->lineEditGaiNian2ZhangDieEnd->text().toFloat(&ok2);

    if (!ok1 || !ok2 || stockSetting->m_zhangDieStart > stockSetting->m_zhangDieEnd)
    {
        UiUtil::showTip(QString::fromWCharArray(L"请正确填写概念2涨跌幅"));
        return;
    }

    ok = false;
    stockSetting->m_kuoJianPercent = ui->lineEditGaiNian2Percent->text().toFloat(&ok);
    if (!ok)
    {
        UiUtil::showTip(QString::fromWCharArray(L"请正确填写值算法中的概念2部分"));
        return;
    }
    stockSetting->m_kuoJianPercent /= 100;

    // 个股相关部分
    stockSetting = &setting->m_stockSetting[STOCK_TYPE_GEGU];
    stockSetting->m_enable = ui->checkBoxGeGuGuPiao->isChecked();
    stockSetting->m_stockPath = ui->lineEditGeGuGuPiao->text();
    if (stockSetting->m_enable)
    {        
        if (!QFile(stockSetting->m_stockPath).exists())
        {
            UiUtil::showTip(QString::fromWCharArray(L"请选择个股股票目录"));
            return;
        }
    }

    stockSetting->m_enableZhangDie = ui->checkBoxGeGuZhangDie->isChecked();
    ok1 = false;
    stockSetting->m_zhangDieStart = ui->lineEditGeGuZhangDieStart->text().toFloat(&ok1);

    ok2 = false;
    stockSetting->m_zhangDieEnd = ui->lineEditGeGuZhangDieEnd->text().toFloat(&ok2);

    if (!ok1 || !ok2 || stockSetting->m_zhangDieStart > stockSetting->m_zhangDieEnd)
    {
        UiUtil::showTip(QString::fromWCharArray(L"请正确填写个股涨跌幅"));
        return;
    }

    // 查找方式
    if (ui->radioButtonJingqueFind->isChecked())
    {
        setting->m_searchMethod = SEARCH_TYPE_JINGQUE;
    }
    else if (ui->radioButtonKeyFind->isChecked())
    {
        setting->m_searchMethod = SEARCH_TYPE_KEY_WORD;
    }
    else if (ui->radioButtonAlgFind->isChecked())
    {
        setting->m_searchMethod = SEARCH_TYPE_ALGORITHM;
    }

    // 对比内容来源
    if (ui->radioButtonZhidingWord->isChecked())
    {
        setting->m_compareContentFrom = COMPARE_CONTENT_ZHIDING_WORD;
        setting->m_zhiDingWordFilterCondition.m_oneInclude = ui->lineEditZhiDingOneGongInclude->text();
        setting->m_zhiDingWordFilterCondition.m_twoInclude = ui->lineEditZhiDingTwoGongInclude->text();
        if (!setting->m_zhiDingWordFilterCondition.isEnable())
        {
            UiUtil::showTip(QString::fromWCharArray(L"请填写指定字的内容"));
            return;
        }
    }
    else if (ui->radioButtonAllWord->isChecked())
    {
        setting->m_compareContentFrom = COMPARE_CONTENT_ALL_WORD;
        setting->m_allWordFilterCondition.m_oneInclude = ui->lineEditAllWordOneGongInclude->text();
        setting->m_allWordFilterCondition.m_oneExclude = ui->lineEditAllWordOneGongExclude->text();
        setting->m_allWordFilterCondition.m_twoInclude = ui->lineEditAllWordTwoGongInclude->text();
        setting->m_allWordFilterCondition.m_twoExclude = ui->lineEditAllWordTwoGongExclude->text();
        if (!setting->m_allWordFilterCondition.isEnable())
        {
            UiUtil::showTip(QString::fromWCharArray(L"请填写全部字的内容"));
            return;
        }
    }
    else if (ui->radioButtonZhidingDate->isChecked())
    {
        setting->m_compareDate = ui->dateEditZhiding->dateTime().toSecsSinceEpoch();
        if (ui->radioButtonDay->isChecked())
        {
            setting->m_compareContentFrom = COMPARE_CONTENT_ZHIDING_DAY;
        }
        else if (ui->radioButtonMonth->isChecked())
        {
            setting->m_compareContentFrom = COMPARE_CONTENT_ZHIDING_MONTH;
        }
        else if (ui->radioButtonYear->isChecked())
        {
            setting->m_compareContentFrom = COMPARE_CONTENT_ZHIDING_YEAR;
        }
    }

    setting->m_allMatch = ui->checkBoxAllMatch->isChecked();
    setting->m_sortDesc = ui->checkBoxSortDecend->isChecked();
    setting->m_recgDateStart = ui->dateEditRecgStart->dateTime().toSecsSinceEpoch();
    setting->m_recgDateEnd= ui->dateEditRecgEnd->dateTime().addDays(1).toSecsSinceEpoch();
    if (setting->m_recgDateStart >= setting->m_recgDateEnd)
    {
        UiUtil::showTip(QString::fromWCharArray(L"请正确填写识别日期范围"));
        return;
    }

    // 精确查找的内容是指定字或从股票获取
    if ((setting->m_searchMethod == SEARCH_TYPE_JINGQUE || setting->m_searchMethod == SEARCH_TYPE_KEY_WORD)
            && setting->m_compareContentFrom == COMPARE_CONTENT_ALL_WORD)
    {
        UiUtil::showTip(QString::fromWCharArray(L"精确查找和关键字查找不能选择全部字"));
        return;
    }

    setting->save();

    startSearch();
}


void MainWindow::startSearch()
{    
    ui->pushButtonStart->setEnabled(false);
    ui->pushButtonSave->setEnabled(false);
    ui->textEditLog->clear();

    MainController* controller = new MainController();
    connect(controller, &MainController::runFinish, [this, controller] () {
        ui->pushButtonStart->setEnabled(true);
        ui->pushButtonSave->setEnabled(true);
        controller->deleteLater();
    });
    connect(controller, &MainController::printLog, this, &MainWindow::onPrintLog);
    connect(controller, &MainController::printResult, this, &MainWindow::onPrintResult);

    controller->run();
}
