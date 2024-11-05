#include "getonevaluedialog.h"
#include "ui_getonevaluedialog.h"
#include "uiutil.h"

GetOneValueDialog::GetOneValueDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::GetOneValueDialog)
{
    ui->setupUi(this);

    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setWindowFlags(windowFlags() & ~Qt::WindowMaximizeButtonHint);
    setWindowFlag(Qt::MSWindowsFixedSizeDialogHint, true);

    connect(ui->okButton, &QPushButton::clicked, [this]() {
        bool ok = false;
        m_value = ui->valueEdit->text().toInt(&ok);
        if (!ok || m_value < -20 || m_value > 20)
        {
            UiUtil::showTip(QString::fromWCharArray(L"值必须是-20到20的值"));
            return;
        }

        accept();
        close();
    });

    connect(ui->cancelButton, &QPushButton::clicked, [this]() {
        close();
    });
}

GetOneValueDialog::~GetOneValueDialog()
{
    delete ui;
}
