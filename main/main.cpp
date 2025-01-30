#include "mainwindow.h"

#include <QApplication>

#include "Utility/LogUtil.h"
#include "Utility/DumpUtil.h"
#include "Utility/ImPath.h"
#include "settingmanager.h"

CLogUtil* g_dllLog = nullptr;

QtMessageHandler originalHandler = nullptr;

void logToFile(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    // 去除无效的日志
    if (msg.indexOf("Could not get the INetworkConnection instance") >= 0)
    {
        return;
    }

    if (g_dllLog)
    {
        ELogLevel logLevel = ELogLevel::LOG_LEVEL_ERROR;
        if (type == QtMsgType::QtDebugMsg)
        {
            logLevel = ELogLevel::LOG_LEVEL_DEBUG;
        }
        else if (type == QtMsgType::QtInfoMsg || type == QtMsgType::QtWarningMsg)
        {
            logLevel = ELogLevel::LOG_LEVEL_INFO;
        }

        QString newMsg = msg;
        newMsg.remove(QChar('%'));
        g_dllLog->Log(context.file? context.file: "", context.line, logLevel, newMsg.toStdWString().c_str());
    }

    if (originalHandler)
    {
        (*originalHandler)(type, context, msg);
    }
}

int main(int argc, char *argv[])
{
    g_dllLog = CLogUtil::GetLog(L"main");

    // 初始化崩溃转储机制
//    CDumpUtil::SetDumpFilePath(CImPath::GetDumpPath().c_str());
//    CDumpUtil::Enable(true);

    // 设置日志级别
    int nLogLevel = SettingManager::getInstance()->m_nLogLevel;
    g_dllLog->SetLogLevel((ELogLevel)nLogLevel);
    originalHandler = qInstallMessageHandler(logToFile);

    qputenv("QT_FONT_DPI", "100");

    QApplication a(argc, argv);    

    MainWindow w;
    w.show();
    return a.exec();
}
