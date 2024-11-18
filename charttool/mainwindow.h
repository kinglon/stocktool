#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "mychartwidget.h"
#include "datamanager.h"

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

    bool parseChartDataLine(const QString& line, ChartData& chartData);

private slots:
    void onLoadDataButtonClicked();

    void onSaveImageButtonClicked();

private:
    Ui::MainWindow *ui;

    MyChartWidget* m_myChartWidget = nullptr;
};
#endif // MAINWINDOW_H
