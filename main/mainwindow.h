#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QShortcut>
#include <QCloseEvent>
#include "settingmanager.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onCtrlDShortcut();

    void onPrintLog(QString content);

    void onLoadDataButtonClicked();

    void onFilterDataButtonClicked();

    void onFilterHourDataButtonClicked();

    void onDayCompareButtonClicked();

    void onMonthCompareButtonClicked();

protected:
    void closeEvent(QCloseEvent *event) override;

private:
    void initCtrls();

    // 控制是否允许操作按钮
    void enableOperate(bool enable);

    // filterHour 筛选已午未
    void doFilter(bool filterHour);

private:
    Ui::MainWindow *ui;

    QShortcut* m_ctrlDShortcut = nullptr;    
};
#endif // MAINWINDOW_H
