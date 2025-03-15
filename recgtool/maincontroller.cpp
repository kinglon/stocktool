#include "maincontroller.h"
#include "../stock/loaddatacontroller.h"
#include "stockdatamanager.h"
#include "filterdatacontroller.h"
#include "stockdatawriter.h"

void WriteDataThread::run()
{
    StockDataWriter writer;
    StockDataManager::getInstance()->m_result = writer.getResult(m_stockDatas);
    emit runFinish();
}

MainController::MainController(QObject *parent)
    : QObject{parent}
{

}

void MainController::run()
{
    bool needLoadData = true;
    if (SettingManager::getInstance()->m_cacheData && StockDataManager::getInstance()->hasData())
    {
        needLoadData = false;
    }

    if (needLoadData)
    {
        StockDataManager::getInstance()->clear();

        // 加载数据
        emit printLog(QString::fromWCharArray(L"开始加载各目录的数据"));
        doLoadData();
    }
    else
    {
        doFilterData();
    }
}

void MainController::doLoadData()
{
    if (m_loadDataNextIndex > STOCK_TYPE_ZHISHU)
    {
        emit printLog(QString::fromWCharArray(L"加载各目录的数据完成"));
        doFilterData();
        return;
    }

    if (!SettingManager::getInstance()->m_stockSetting[m_loadDataNextIndex].m_enable)
    {
        m_loadDataNextIndex++;
        doLoadData();
        return;
    }

    LoadDataController* controller = new LoadDataController(this);
    controller->setDataManager(StockDataManager::getInstance()->m_dataManagers[m_loadDataNextIndex]);
    connect(controller, &LoadDataController::printLog, this, &MainController::printLog);
    connect(controller, &LoadDataController::runFinish, [this, controller]() {
        controller->deleteLater();
        m_loadDataNextIndex++;
        doLoadData();
    });
    controller->run(SettingManager::getInstance()->m_stockSetting[m_loadDataNextIndex].m_stockPath);
}

void MainController::doFilterData()
{
    emit printLog(QString::fromWCharArray(L"开始识别数据"));
    FilterDataController* controller = new FilterDataController(this);
    connect(controller, &FilterDataController::printLog, this, &MainController::printLog);
    connect(controller, &FilterDataController::runFinish, [this, controller]() {
        emit printLog(QString::fromWCharArray(L"识别数据完成"));
        if (controller->m_stockDatas.empty())
        {
            emit printLog(QString::fromWCharArray(L"没有识别到数据"));
            emit runFinish();
            return;
        }

        doWriteData(controller->m_stockDatas);
        controller->deleteLater();
    });
    controller->run();
}

void MainController::doWriteData(const QVector<StockData>& stockDatas)
{
    emit printLog(QString::fromWCharArray(L"开始生成结果"));
    WriteDataThread* thread = new WriteDataThread();
    thread->m_stockDatas = stockDatas;
    connect(thread, &QThread::finished, thread, &QThread::deleteLater);
    connect(thread, &WriteDataThread::runFinish, this, &MainController::onWriteDataFinish);
    thread->start();
}

void MainController::onWriteDataFinish()
{
    if (StockDataManager::getInstance()->m_result.isEmpty())
    {
        emit printResult(QString::fromWCharArray(L"没有数据"));
    }
    else
    {
        emit printResult(StockDataManager::getInstance()->m_result);
    }

    emit runFinish();
}
