#ifndef NOTLOSSFILTERDIALOG_H
#define NOTLOSSFILTERDIALOG_H

#include <QDialog>
#include <QDate>
#include "../stock/stockdatautil.h"

namespace Ui {
class NotLossFilterDialog;
}

class NotLossFilterDialog : public QDialog
{
    Q_OBJECT

public:
    explicit NotLossFilterDialog(QWidget *parent = nullptr);
    ~NotLossFilterDialog();

private:
    void initCtrls();

    void onOkBtn();

private:
    Ui::NotLossFilterDialog *ui;

public:
    int m_notLossFilterDataType = STOCK_DATA_YEAR;

    // 过滤时间范围
    QDate m_beginDate;
    QDate m_endDate; // 不含
};

#endif // NOTLOSSFILTERDIALOG_H
