#include "notlossfilterdialog.h"
#include "ui_notlossfilterdialog.h"
#include "uiutil.h"
#include "settingmanager.h"

NotLossFilterDialog::NotLossFilterDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::NotLossFilterDialog)
{
    ui->setupUi(this);

    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setWindowModality(Qt::WindowModal);
    setWindowFlags(windowFlags() & ~Qt::WindowMaximizeButtonHint);
    setWindowFlag(Qt::MSWindowsFixedSizeDialogHint, true);

    initCtrls();
}

NotLossFilterDialog::~NotLossFilterDialog()
{
    delete ui;
}

void NotLossFilterDialog::initCtrls()
{
    ui->dataTypeComboBox->clear();
    ui->dataTypeComboBox->addItem(QString::fromWCharArray(L"年"), QVariant(STOCK_DATA_YEAR));
    ui->dataTypeComboBox->addItem(QString::fromWCharArray(L"月"), QVariant(STOCK_DATA_MONTH));
    ui->dataTypeComboBox->addItem(QString::fromWCharArray(L"日"), QVariant(STOCK_DATA_DAY));
    ui->dataTypeComboBox->addItem(QString::fromWCharArray(L"时"), QVariant(STOCK_DATA_HOUR));
    ui->dataTypeComboBox->setCurrentIndex(0);

    ui->beginDateEdit->setDate(QDate::currentDate().addYears(-1));
    ui->endDateEdit->setDate(QDate::currentDate());

    connect(ui->cancelButton, &QPushButton::clicked, [this] () {
        close();
    });
    connect(ui->okButton, &QPushButton::clicked, [this] () {
        onOkBtn();
    });
}

void NotLossFilterDialog::onOkBtn()
{
    FilterConditionV2 filterConditionV2;
    filterConditionV2.m_filterItems[0].m_include = ui->conditionEdit1->text();
    filterConditionV2.m_filterItems[0].m_exclude = ui->conditionEdit2->text();
    filterConditionV2.m_filterItems[1].m_include = ui->conditionEdit3->text();
    filterConditionV2.m_filterItems[1].m_exclude = ui->conditionEdit4->text();
    filterConditionV2.m_filterItems[2].m_include = ui->conditionEdit5->text();
    filterConditionV2.m_filterItems[2].m_exclude = ui->conditionEdit6->text();
    filterConditionV2.m_filterItems[3].m_include = ui->conditionEdit7->text();
    filterConditionV2.m_filterItems[3].m_exclude = ui->conditionEdit8->text();
    filterConditionV2.m_filterItems[4].m_include = ui->conditionEdit9->text();
    filterConditionV2.m_filterItems[4].m_exclude = ui->conditionEdit10->text();
    filterConditionV2.m_filterItems[5].m_include = ui->conditionEdit11->text();
    filterConditionV2.m_filterItems[5].m_exclude = ui->conditionEdit12->text();
    if (!filterConditionV2.isEnable())
    {
        UiUtil::showTip(QString::fromWCharArray(L"请配置关键词"));
        return;
    }

    QDate beginDate = ui->beginDateEdit->date();
    QDate endDate = ui->endDateEdit->date().addDays(1); // 筛选的时候不含endDate这一天
    if (beginDate > endDate)
    {
        UiUtil::showTip(QString::fromWCharArray(L"请正确设置筛选时间"));
        return;
    }

    SettingManager::getInstance()->m_filterConditionV2 = filterConditionV2;
    m_beginDate = beginDate;
    m_endDate = endDate;
    m_notLossFilterDataType = ui->dataTypeComboBox->currentData().toInt();

    accept();
}
