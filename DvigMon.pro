QT       += core gui \
            serialport \
            charts




greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += \
    chart.cpp \
    chartview.cpp \
    chartview_move.cpp \
    getparams.cpp \
    main.cpp \
    moveitem.cpp \
    widget.cpp

HEADERS += \
    chart.h \
    chartview.h \
    chartview_move.h \
    getparams.h \
    moveitem.h \
    qchartview.h \
    ui_widget.h \
    widget.h

FORMS += \
    widget.ui

QMAKE_LFLAGS += -static -static-libgcc

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
