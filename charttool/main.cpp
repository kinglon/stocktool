#include "mainwindow.h"

#include <QApplication>
#include "Utility/LogUtil.h"

CLogUtil* g_dllLog = nullptr;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
