#ifndef MAINCONTROLLER_H
#define MAINCONTROLLER_H

#include <QObject>

class MainController : public QObject
{
    Q_OBJECT
public:
    explicit MainController(QObject *parent = nullptr);

    void run();

signals:
    void runFinish();

    void printLog(QString content);

    void printResult(QString result);
};

#endif // MAINCONTROLLER_H
