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

    bool parseColorDataLine(const QString& line, ColorData& colorData, const QString& oneInclude, const QString& twoInclude);

    void checkIfInclude(QString data[4], const QString& oneInclude, const QString& twoInclude, bool& isOneInclude, bool& isTwoInclude);

    bool hasCunWord(QString data, QString data1, QString data2);

    bool haveWordWithoutKuohao(QString word, QString data[4]);

private slots:
    // 加载主能量数据
    void onLoadDataButtonClicked();

    // 加载辅助能量1数据
    void onLoadAssist1DataButtonClicked();

    // 加载辅助能量2数据
    void onLoadAssist2DataButtonClicked();

    // 加载主色块数据
    void onLoadColorDataButtonClicked();

    // 加载辅1色块数据
    void onLoadAssist1ColorDataButtonClicked();

    // 加载辅2色块数据
    void onLoadAssist2ColorDataButtonClicked();

    void onSaveImageButtonClicked();

    void onAddAvgLineButtonClicked();

private:
    Ui::MainWindow *ui;

    MyChartWidget* m_myChartWidget = nullptr;
};
#endif // MAINWINDOW_H
