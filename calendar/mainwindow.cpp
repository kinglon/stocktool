#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "mycalendarwidget.h"
#include <QVBoxLayout>

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
    QVBoxLayout *layout = new QVBoxLayout(ui->centralwidget);
    QCalendarWidget *calendarWidget = new MyCalendarWidget(this);
    layout->addWidget(calendarWidget);
}
