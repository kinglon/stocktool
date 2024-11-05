#ifndef GETONEVALUEDIALOG_H
#define GETONEVALUEDIALOG_H

#include <QDialog>

namespace Ui {
class GetOneValueDialog;
}

class GetOneValueDialog : public QDialog
{
    Q_OBJECT

public:
    explicit GetOneValueDialog(QWidget *parent = nullptr);
    ~GetOneValueDialog();

public:
    int m_value = 0;

private:
    Ui::GetOneValueDialog *ui;
};

#endif // GETONEVALUEDIALOG_H
