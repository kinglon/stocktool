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
    update();
}

void MyChartWidget::paintEvent(QPaintEvent *)
{
    QPainter painter(this);

    // 画背景
    painter.setBrush(Qt::white);
    painter.setPen(Qt::white);
    painter.drawRect(QRect(0, 0, width(), height()));

    // 一行行画柱条
    const int maxWidth = width() - DATE_AREA_WIDTH - BAR_RIGHT_PADDING;
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
        int lineWidth = maxWidth;
        if (chartData.m_count < m_maxCount)
        {
            lineWidth = (int)(chartData.m_count*1.0f/m_maxCount*maxWidth);
        }
        static int topPadding = (LINE_HEIGHT-BAR_HEIGHT)/2;
        QRect rectToDraw(DATE_AREA_WIDTH, i*LINE_HEIGHT+topPadding, lineWidth, BAR_HEIGHT);
        QColor barColor(chartData.m_color);
        painter.setBrush(QBrush(barColor));
        painter.setPen(barColor);
        painter.drawRect(rectToDraw);
    }

    // 一条条画均线
    for (int i=0; i<DataManager::getInstance()->m_avgLines.size(); i++)
    {
        AvgLine& avgLine = DataManager::getInstance()->m_avgLines[i];
        QRect rectToDraw;
        rectToDraw.setLeft(DATE_AREA_WIDTH+maxWidth);
        if (avgLine.m_count < m_maxCount)
        {
            rectToDraw.setLeft(DATE_AREA_WIDTH+(int)(avgLine.m_count*1.0f/m_maxCount*maxWidth));
        }
        rectToDraw.setRight(rectToDraw.left()+1);
        rectToDraw.setTop(avgLine.m_begin*LINE_HEIGHT);
        rectToDraw.setBottom((avgLine.m_end+1)*LINE_HEIGHT);
        painter.setBrush(Qt::green);
        painter.setPen(Qt::green);
        painter.drawRect(rectToDraw);
    }
}

void MyChartWidget::mousePressEvent(QMouseEvent *event)
{
    QWidget::mousePressEvent(event);

    // 获取鼠标单击的坐标（相对于 MyWidget）
    QPoint clickPos = event->pos();
    int lineIndex = clickPos.y() / LINE_HEIGHT;
    if (event->button() == Qt::LeftButton)
    {
        if (lineIndex >= 0 && lineIndex < DataManager::getInstance()->m_chartDatas.size())
        {
            DataManager::getInstance()->m_chartDatas[lineIndex].switchNextColor();
            update();
        }
    }
    else if (event->button() == Qt::RightButton)
    {
        for (int i=0; i<DataManager::getInstance()->m_avgLines.size(); i++)
        {
            AvgLine& avgLine = DataManager::getInstance()->m_avgLines[i];
            if (lineIndex >= avgLine.m_begin && lineIndex <= avgLine.m_end)
            {
                DataManager::getInstance()->m_avgLines.remove(i);
                update();
                break;
            }
        }
    }
}
