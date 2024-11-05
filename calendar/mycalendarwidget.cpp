#include "mycalendarwidget.h"
#include <QPainter>
#include <QDateTime>
#include "datamanager.h"
#include "getonevaluedialog.h"

MyCalendarWidget::MyCalendarWidget(QWidget *parent)
    : QCalendarWidget(parent)
{
    connect(this, &MyCalendarWidget::clicked, [this](const QDate &date){
        GetOneValueDialog dlg(this->window());
        dlg.show();
        if (dlg.exec() == QDialog::Accepted)
        {
            QDateTime dateTime;
            dateTime.setDate(date);
            DataManager::getInstance()->addValue(dateTime.toSecsSinceEpoch(), dlg.m_value);
            this->update();
        }
    });
}

void MyCalendarWidget::paintCell(QPainter *painter, const QRect &rect, const QDate &date) const
{
    // 画边框
    painter->setBrush(Qt::NoBrush);
    painter->setPen(Qt::gray);
    painter->drawRect(rect);

    // 画日期
    painter->setPen(Qt::black);
    QString dayText = QString::number(date.day());
    painter->drawText(rect.x()+10, rect.y()+rect.height()/2+4, dayText);

    // 画条形
    static int padding = 3;
    static int maxValue = 20;
    QDateTime dateTime;
    dateTime.setDate(date);
    int currentValue = DataManager::getInstance()->getValue(dateTime.toSecsSinceEpoch());
    if (currentValue != 0)
    {
        int currentHeight = int((qAbs(currentValue) * 1.0f / maxValue) * (rect.height()-padding*2));
        QRect rectToDraw(rect.right()-10, rect.bottom()-padding-currentHeight, 5, currentHeight);
        if (currentValue > 0)
        {
            painter->setBrush(Qt::red);
        }
        else
        {
            painter->setBrush(Qt::green);
        }
        painter->drawRect(rectToDraw);
    }
}
