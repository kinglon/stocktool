#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

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

    // 控制是否允许操作按钮
    void enableOperate(bool enable);

private slots:
    void onPrintLog(QString content);

    void onLoadDataButtonClicked();

    void onScoreButtonClicked();

    void onExportButtonClicked();

private:
    Ui::MainWindow *ui;

    QString m_loadDataPath;

    QString m_scoreResult;
};
#endif // MAINWINDOW_H
