#ifndef MYCALENDARWIDGET_H
#define MYCALENDARWIDGET_H

#include <QCalendarWidget>

class MyCalendarWidget : public QCalendarWidget
{
    Q_OBJECT
public:
    MyCalendarWidget(QWidget *parent = nullptr);

protected:
    void paintCell(QPainter *painter, const QRect &rect, const QDate &date) const override;
};

#endif // MYCALENDARWIDGET_H
