QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    Utility/DumpUtil.cpp \
    Utility/IcrCriticalSection.cpp \
    Utility/ImCharset.cpp \
    Utility/ImPath.cpp \
    Utility/LogBuffer.cpp \
    Utility/LogUtil.cpp \
    datamanager.cpp \
    datamodel.cpp \
    daymergedatacontroller.cpp \
    filterdatacontroller.cpp \
    loaddatacontroller.cpp \
    main.cpp \
    mainwindow.cpp \
    mergedatacontroller.cpp \
    monthmergedatacontroller.cpp \
    notlossfilterdialog.cpp \
    settingmanager.cpp \
    stockdatautil.cpp \
    uiutil.cpp

HEADERS += \
    Utility/DumpUtil.h \
    Utility/IcrCriticalSection.h \
    Utility/ImCharset.h \
    Utility/ImPath.h \
    Utility/LogBuffer.h \
    Utility/LogMacro.h \
    Utility/LogUtil.h \
    datamanager.h \
    datamodel.h \
    daymergedatacontroller.h \
    filterdatacontroller.h \
    loaddatacontroller.h \
    mainwindow.h \
    mergedatacontroller.h \
    monthmergedatacontroller.h \
    notlossfilterdialog.h \
    settingmanager.h \
    stockdatautil.h \
    uiutil.h

FORMS += \
    mainwindow.ui \
    notlossfilterdialog.ui

# Enable PDB generation
QMAKE_CFLAGS_RELEASE += /Zi
QMAKE_CXXFLAGS_RELEASE += /Zi
QMAKE_LFLAGS_RELEASE += /DEBUG

# Enable log context
DEFINES += QT_MESSAGELOGCONTEXT
