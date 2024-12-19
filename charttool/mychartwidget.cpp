#include "mychartwidget.h"
#include <QPainter>
#include <QDateTime>
#include <QMouseEvent>
#include "datamanager.h"

// 一行的高度
#define LINE_HEIGHT     26

// 柱状图右边padding
#define BAR_RIGHT_PADDING 55

// 柱状图上边padding
#define BAR_TOP_PADDING 3

// 柱状图下边padding
#define BAR_BOTTOM_PADDING 3

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
        QRect barRect(DATE_AREA_WIDTH, i*LINE_HEIGHT+BAR_TOP_PADDING,
                      maxWidth, LINE_HEIGHT-BAR_TOP_PADDING-BAR_BOTTOM_PADDING);
        paintBar(painter, chartData, barRect);
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

void MyChartWidget::paintBar(QPainter& painter, const ChartData& chartData, const QRect& rect)
{
    // 获取色块数据
    const ColorData* colorData = nullptr;
    for (const auto& item : DataManager::getInstance()->m_colorDatas)
    {
        if (item.m_type == chartData.m_type && item.m_date == chartData.m_date)
        {
            colorData = &item;
            break;
        }
    }

    // 画辅助能量柱1
    const ChartData* assistData1 = nullptr;
    for (const auto& item : DataManager::getInstance()->m_assist1Datas)
    {
        if (item.m_type == chartData.m_type && item.m_date == chartData.m_date)
        {
            assistData1 = &item;
            break;
        }
    }

    int lineWidth = rect.width();
    if (assistData1)
    {
        if (assistData1->m_count == 0)
        {
            lineWidth = rect.width() / m_maxAssist1Count;
            painter.setBrush(QBrush(RGB_ORANGE));
            painter.setPen(RGB_ORANGE);
        }
        else
        {
            if (assistData1->m_count < m_maxAssist1Count)
            {
                lineWidth = (int)(assistData1->m_count*1.0f/m_maxAssist1Count*rect.width());
            }
            QColor color = QColor(173, 131, 241).rgb();
            painter.setBrush(QBrush(color));
            painter.setPen(color);
        }
        QRect assist1Rect(rect.left(), rect.top(), lineWidth, (int)(0.25f*rect.height()));
        painter.drawRect(assist1Rect);

        // 画色块
        int right = assist1Rect.right();
        if (colorData && colorData->oneInclude)
        {
            painter.setBrush(QBrush(RGB_GREEN));
            painter.setPen(RGB_GREEN);
            QRect color1Rect(right, assist1Rect.top(), assist1Rect.height(), assist1Rect.height());
            right += assist1Rect.height();
            painter.drawRect(color1Rect);
        }

        if (colorData && colorData->twoInclude)
        {
            painter.setBrush(QBrush(RGB_ORANGE));
            painter.setPen(RGB_ORANGE);
            QRect color2Rect(right, assist1Rect.top(), assist1Rect.height(), assist1Rect.height());
            right += assist1Rect.height();
            painter.drawRect(color2Rect);
        }
    }

    // 画主能量柱
    lineWidth = rect.width();
    if (chartData.m_count < m_maxCount)
    {
        lineWidth = (int)(chartData.m_count*1.0f/m_maxCount*rect.width());
    }
    QColor barColor(chartData.m_color);
    painter.setBrush(QBrush(barColor));
    painter.setPen(barColor);
    QRect mainRect(rect.left(), rect.top()+(int)(0.25f*rect.height()), lineWidth, (int)(0.5f*rect.height()));
    painter.drawRect(mainRect);

    // 画主能量柱色块
    int right = mainRect.right();
    if (colorData && colorData->oneInclude)
    {
        painter.setBrush(QBrush(RGB_GREEN));
        painter.setPen(RGB_GREEN);
        QRect color1Rect(right, mainRect.top(), mainRect.height(), mainRect.height());
        right += mainRect.height();
        painter.drawRect(color1Rect);
    }

    if (colorData && colorData->twoInclude)
    {
        painter.setBrush(QBrush(RGB_ORANGE));
        painter.setPen(RGB_ORANGE);
        QRect color2Rect(right, mainRect.top(), mainRect.height(), mainRect.height());
        right += mainRect.height();
        painter.drawRect(color2Rect);
    }

    // 画辅助能量柱2
    const ChartData* assistData2 = nullptr;
    for (const auto& item : DataManager::getInstance()->m_assist2Datas)
    {
        if (item.m_type == chartData.m_type && item.m_date == chartData.m_date)
        {
            assistData2 = &item;
            break;
        }
    }

    lineWidth = rect.width();
    if (assistData2)
    {
        if (assistData2->m_count == 0)
        {
            lineWidth = rect.width() / m_maxAssist2Count;
            painter.setBrush(QBrush(RGB_ORANGE));
            painter.setPen(RGB_ORANGE);
        }
        else
        {
            if (assistData2->m_count < m_maxAssist2Count)
            {
                lineWidth = (int)(assistData2->m_count*1.0f/m_maxAssist2Count*rect.width());
            }
            QColor color = QColor(173, 227, 126).rgb();
            painter.setBrush(QBrush(color));
            painter.setPen(color);
        }

        QRect assist2Rect(rect.left(), rect.top()+(int)(0.75f*rect.height()), lineWidth, (int)(0.25f*rect.height()));
        painter.drawRect(assist2Rect);

        // 画色块
        int right = assist2Rect.right();
        if (colorData && colorData->oneInclude)
        {
            painter.setBrush(QBrush(RGB_GREEN));
            painter.setPen(RGB_GREEN);
            QRect color1Rect(right, assist2Rect.top(), assist2Rect.height(), assist2Rect.height());
            right += assist2Rect.height();
            painter.drawRect(color1Rect);
        }

        if (colorData && colorData->twoInclude)
        {
            painter.setBrush(QBrush(RGB_ORANGE));
            painter.setPen(RGB_ORANGE);
            QRect color2Rect(right, assist2Rect.top(), assist2Rect.height(), assist2Rect.height());
            right += assist2Rect.height();
            painter.drawRect(color2Rect);
        }
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
