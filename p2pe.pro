TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt
CONFIG(debug, release|debug):DEFINES += _DEBUG

QMAKE_CXXFLAGS += -std=gnu++11

SOURCES += main.cpp \
    client.cpp \
    order.cpp \
    liquidation.cpp

HEADERS += \
    client.h \
    order.h \
    common.h \
    liquidation.h

