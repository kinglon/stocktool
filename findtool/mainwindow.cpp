#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "settingmanager.h"
#include "datamanager.h"
#include "uiutil.h"
#include <QFileDialog>
#include <QDesktopServices>
#include "contentbrowser.h"

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
    ui->originDateEdit->setDate(now);

    ui->afterYearEdit->setText(QString::number(SettingManager::getInstance()->m_afterYear));
    ui->afterMonthEdit->setText(QString::number(SettingManager::getInstance()->m_afterMonth));
    ui->afterDayEdit->setText(QString::number(SettingManager::getInstance()->m_afterDay));
    ui->afterHourEdit->setText(QString::number(SettingManager::getInstance()->m_afterHour));
    ui->beforeYearEdit->setText(QString::number(SettingManager::getInstance()->m_beforeYear));
    ui->beforeMonthEdit->setText(QString::number(SettingManager::getInstance()->m_beforeMonth));
    ui->beforeDayEdit->setText(QString::number(SettingManager::getInstance()->m_beforeDay));
    ui->beforeHourEdit->setText(QString::number(SettingManager::getInstance()->m_beforeHour));

    connect(ui->selectRootPathButton, &QPushButton::clicked, this, &MainWindow::onSelectRootPathButtonClicked);
    connect(ui->stockFolderButton, &QPushButton::clicked, this, &MainWindow::onStockFolderButtonClicked);
    connect(ui->stockBrowseButton, &QPushButton::clicked, this, &MainWindow::onStockBrowseButtonClicked);
    connect(ui->stockFolderList, &QListWidget::itemClicked, this, [](QListWidgetItem *item) {
        QDesktopServices::openUrl(QUrl::fromLocalFile(item->data(Qt::UserRole).toString()));
    });
    connect(ui->stockBrowserList, &QListWidget::itemClicked, this, &MainWindow::onStockBrowserItemClicked);
}

void MainWindow::onSelectRootPathButtonClicked()
{
    QString rootDir = QFileDialog::getExistingDirectory(this, QString::fromWCharArray(L"选择目录"),
                                                          QDir::homePath());
    if (rootDir.isEmpty())
    {
        return;
    }

    ui->rootPathEdit->setText(rootDir);

    DataManager::getInstance()->m_stocks.clear();

    QDir dir(rootDir);
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
                Stock stock;
                stock.m_stockPath = fileInfo.absoluteFilePath();
                stock.m_stockName = fileInfo.fileName();
                stock.m_stockId = getStockId(stock.m_stockPath);
                DataManager::getInstance()->m_stocks.append(stock);
                break;
            }
        }

        // 行业下查找
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
                        Stock stock;
                        stock.m_stockPath = subFileInfo.absoluteFilePath();
                        stock.m_stockName = subFileInfo.fileName();
                        stock.m_stockId = getStockId(stock.m_stockPath);
                        DataManager::getInstance()->m_stocks.append(stock);
                    }
                }
            }
        }
    }

    UiUtil::showTip(QString::fromWCharArray(L"股票总数%1").arg(DataManager::getInstance()->m_stocks.size()));
}

QString MainWindow::getStockId(const QString& stockPath)
{
    QDir dir(stockPath);
    QDir::Filters filters = QDir::Files;
    QFileInfoList fileInfoList = dir.entryInfoList(filters);
    foreach (const QFileInfo &fileInfo, fileInfoList)
    {
        if (fileInfo.fileName().contains(QString::fromWCharArray(L"日线")))
        {
            QStringList parts = fileInfo.fileName().split('_');
            if (parts.size() >= 2)
            {
                return parts[1];
            }
            break;
        }
    }

    return "";
}

void MainWindow::onStockFolderButtonClicked()
{
    ui->stockFolderList->clear();
    QString keyWord = ui->stockFolderEdit->text();
    if (keyWord.isEmpty())
    {
        return;
    }

    foreach(const Stock& stock, DataManager::getInstance()->m_stocks)
    {
        if (stock.m_stockName.contains(keyWord) || stock.m_stockId.contains(keyWord))
        {
            QListWidgetItem* item = new QListWidgetItem(stock.m_stockName+stock.m_stockId);
            item->setData(Qt::UserRole, stock.m_stockPath);
            ui->stockFolderList->addItem(item);
        }
    }
}

void MainWindow::onStockBrowseButtonClicked()
{
    ui->stockBrowserList->clear();
    QString keyWord = ui->stockBrowserEdit->text();
    if (keyWord.isEmpty())
    {
        return;
    }

    foreach(const Stock& stock, DataManager::getInstance()->m_stocks)
    {
        if (stock.m_stockName.contains(keyWord) || stock.m_stockId.contains(keyWord))
        {
            QListWidgetItem* item = new QListWidgetItem(stock.m_stockName+stock.m_stockId);
            item->setData(Qt::UserRole, stock.m_stockPath);
            ui->stockBrowserList->addItem(item);
        }
    }
}

void MainWindow::onStockBrowserItemClicked(QListWidgetItem* item)
{
    if (m_contentBrowser)
    {
        UiUtil::showTip(QString::fromWCharArray(L"正在查看内容"));
        return;
    }

    bool ok = false;
    int value = ui->afterYearEdit->text().toInt(&ok);
    if (!ok || value < 0)
    {
        UiUtil::showTip(QString::fromWCharArray(L"请正确填写显示设置"));
        return;
    }
    SettingManager::getInstance()->m_afterYear = value;

    value = ui->afterMonthEdit->text().toInt(&ok);
    if (!ok || value < 0)
    {
        UiUtil::showTip(QString::fromWCharArray(L"请正确填写显示设置"));
        return;
    }
    SettingManager::getInstance()->m_afterMonth = value;

    value = ui->afterDayEdit->text().toInt(&ok);
    if (!ok || value < 0)
    {
        UiUtil::showTip(QString::fromWCharArray(L"请正确填写显示设置"));
        return;
    }
    SettingManager::getInstance()->m_afterDay = value;

    value = ui->afterHourEdit->text().toInt(&ok);
    if (!ok || value < 0)
    {
        UiUtil::showTip(QString::fromWCharArray(L"请正确填写显示设置"));
        return;
    }
    SettingManager::getInstance()->m_afterHour = value;

    value = ui->beforeYearEdit->text().toInt(&ok);
    if (!ok || value < 0)
    {
        UiUtil::showTip(QString::fromWCharArray(L"请正确填写显示设置"));
        return;
    }
    SettingManager::getInstance()->m_beforeYear = value;

    value = ui->beforeMonthEdit->text().toInt(&ok);
    if (!ok || value < 0)
    {
        UiUtil::showTip(QString::fromWCharArray(L"请正确填写显示设置"));
        return;
    }
    SettingManager::getInstance()->m_beforeMonth = value;

    value = ui->beforeDayEdit->text().toInt(&ok);
    if (!ok || value < 0)
    {
        UiUtil::showTip(QString::fromWCharArray(L"请正确填写显示设置"));
        return;
    }
    SettingManager::getInstance()->m_beforeDay = value;

    value = ui->beforeHourEdit->text().toInt(&ok);
    if (!ok || value < 0)
    {
        UiUtil::showTip(QString::fromWCharArray(L"请正确填写显示设置"));
        return;
    }
    SettingManager::getInstance()->m_beforeHour = value;

    SettingManager::getInstance()->save();

    ui->browserTextEdit->clear();
    QString stockPath = item->data(Qt::UserRole).toString();
    QDate originDate = ui->originDateEdit->date();
    m_contentBrowser = new ContentBrowser();
    m_contentBrowser->m_stockPath = stockPath;
    m_contentBrowser->m_originDate = originDate;
    QThread* thread = new QThread();
    m_contentBrowser->moveToThread(thread);
    connect(thread, &QThread::started, m_contentBrowser, &ContentBrowser::run);
    connect(thread, &QThread::finished, thread, &QThread::deleteLater);
    connect(m_contentBrowser, &ContentBrowser::printContent, this, &MainWindow::onPrintContent);
    connect(m_contentBrowser, &ContentBrowser::runFinish, this, &MainWindow::onContentBrowserFinish);
    thread->start();
}

void MainWindow::onPrintContent(QString content)
{
    ui->browserTextEdit->appendPlainText(content);
}

void MainWindow::onContentBrowserFinish()
{
    if (m_contentBrowser)
    {
        m_contentBrowser->deleteLater();
        m_contentBrowser = nullptr;
    }
}
