#include "mychartwidget.h"
#include <QPainter>
#include <QDateTime>
#include <QMouseEvent>
#include "datamanager.h"

// 一行的高度
#define LINE_HEIGHT     20

// 柱状图高度
#define BAR_HEIGHT (LINE_HEIGHT/2)

// 柱状图右边padding
#define BAR_RIGHT_PADDING 30

// 日期显示的矩形宽度
#define DATE_AREA_WIDTH  100

MyChartWidget::MyChartWidget(QWidget *parent)
    : QWidget{parent}
{

}

void MyChartWidget::onDataChanged()
{
    setFixedHeight(DataManager::getInstance()->m_chartDatas.size() * LINE_HEIGHT);
}

void MyChartWidget::paintEvent(QPaintEvent *)
{
    QPainter painter(this);

    // 画背景
    painter.setBrush(Qt::white);
    painter.setPen(Qt::white);
    painter.drawRect(QRect(0, 0, width(), height()));

    // 一行行画数据
    for (int i=0; i<DataManager::getInstance()->m_chartDatas.size(); i++)
    {
        ChartData& chartData = DataManager::getInstance()->m_chartDatas[i];

        // 画日期
        QDateTime dayTime = QDateTime::fromSecsSinceEpoch(chartData.m_date);
        QString dateString = dayTime.toString("yyyy-MM-dd");
        if (chartData.m_type == CHART_DATA_TYPE_MONTH)
        {
            dateString = dayTime.toString("yyyy-MM");
        }
        painter.setPen(Qt::black);
        painter.drawText(10, i*LINE_HEIGHT, DATE_AREA_WIDTH, LINE_HEIGHT,
                                 Qt::AlignLeft | Qt::AlignVCenter, dateString);

        // 画柱状图
        static int maxCount = 30;
        int maxWidth = width() - DATE_AREA_WIDTH - BAR_RIGHT_PADDING;
        int lineWidth = maxWidth;
        if (chartData.m_count < maxCount)
        {
            lineWidth = (int)(chartData.m_count*1.0f/maxCount*maxWidth);
        }
        static int topPadding = (LINE_HEIGHT-BAR_HEIGHT)/2;
        QRect rectToDraw(DATE_AREA_WIDTH, i*LINE_HEIGHT+topPadding, lineWidth, BAR_HEIGHT);
        QColor barColor(chartData.m_color);
        painter.setBrush(QBrush(barColor));
        painter.setPen(barColor);
        painter.drawRect(rectToDraw);
    }
}

void MyChartWidget::mousePressEvent(QMouseEvent *event)
{
    QWidget::mousePressEvent(event);

    // 获取鼠标单击的坐标（相对于 MyWidget）
    QPoint clickPos = event->pos();
    int lineIndex = clickPos.y() / LINE_HEIGHT;
    if (lineIndex >= 0 && lineIndex < DataManager::getInstance()->m_chartDatas.size())
    {
        DataManager::getInstance()->m_chartDatas[lineIndex].switchNextColor();
        update();
    }
}
