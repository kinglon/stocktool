#ifndef MYCHARTWIDGET_H
#define MYCHARTWIDGET_H

#include <QWidget>

class MyChartWidget : public QWidget
{
    Q_OBJECT
public:
    explicit MyChartWidget(QWidget *parent = nullptr);

public:
    void setMaxCount(int maxCount) { m_maxCount = maxCount; }
    void onDataChanged();

protected:
    void paintEvent(QPaintEvent *event) override;

    void mousePressEvent(QMouseEvent *event) override;

private:
    int m_maxCount = 30;
};

#endif // MYCHARTWIDGET_H
