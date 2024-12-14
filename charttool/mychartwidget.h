#ifndef MYCHARTWIDGET_H
#define MYCHARTWIDGET_H

#include <QWidget>
#include "datamanager.h"

class MyChartWidget : public QWidget
{
    Q_OBJECT
public:
    explicit MyChartWidget(QWidget *parent = nullptr);

public:
    void setMaxCount(int maxCount) { m_maxCount = maxCount; }

    void setMaxAssist1Count(int maxCount) { m_maxAssist1Count = maxCount; }

    void setMaxAssist2Count(int maxCount) { m_maxAssist2Count = maxCount; }

    void onDataChanged();

protected:
    void paintEvent(QPaintEvent *event) override;

    void mousePressEvent(QMouseEvent *event) override;

private:
    void paintBar(QPainter& painter, const ChartData& chartData, const QRect& rect);

private:
    // 主能量柱，满柱
    int m_maxCount = 30;

    // 辅助1能量柱，满柱
    int m_maxAssist1Count = 30;

    // 辅助2能量柱，满柱
    int m_maxAssist2Count = 30;
};

#endif // MYCHARTWIDGET_H
