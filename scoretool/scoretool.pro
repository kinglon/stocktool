QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    ../Utility/IcrCriticalSection.cpp \
    ../Utility/ImCharset.cpp \
    ../Utility/ImPath.cpp \
    ../Utility/LogBuffer.cpp \
    ../Utility/LogUtil.cpp \
    ../Utility/uiutil.cpp \
    ../stock/datamanager.cpp \
    ../stock/loaddatacontroller.cpp \
    ../stock/stockdatautil.cpp \
    main.cpp \
    mainwindow.cpp \
    scorecontroller.cpp \
    settingmanager.cpp

HEADERS += \
    ../Utility/IcrCriticalSection.h \
    ../Utility/ImCharset.h \
    ../Utility/ImPath.h \
    ../Utility/LogBuffer.h \
    ../Utility/LogMacro.h \
    ../Utility/LogUtil.h \
    ../Utility/uiutil.h \
    ../stock/datamanager.h \
    ../stock/loaddatacontroller.h \
    ../stock/stockdatautil.h \
    mainwindow.h \
    scorecontroller.h \
    settingmanager.h

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
