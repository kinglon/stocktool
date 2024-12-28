#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QListWidgetItem>
#include "contentbrowser.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    void initCtrls();

    QString getStockId(const QString& stockPath);

    QString getBrowseContent(const QString& stockPath, const QDate& originDate);

private slots:
    void onSelectRootPathButtonClicked();

    void onScanStockButtonClicked();

    void onStockFolderButtonClicked();

    void onStockBrowseButtonClicked();

    void onStockBrowserItemClicked(QListWidgetItem* item);

    void onPrintContent(QString content);

    void onContentBrowserFinish();

private:
    Ui::MainWindow *ui;

    ContentBrowser* m_contentBrowser = nullptr;
};
#endif // MAINWINDOW_H
