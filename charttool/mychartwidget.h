#ifndef MYCHARTWIDGET_H
#define MYCHARTWIDGET_H

#include <QWidget>

class MyChartWidget : public QWidget
{
    Q_OBJECT
public:
    explicit MyChartWidget(QWidget *parent = nullptr);

public:
    void onDataChanged();

protected:
    void paintEvent(QPaintEvent *event) override;

    void mousePressEvent(QMouseEvent *event) override;
};

#endif // MYCHARTWIDGET_H
