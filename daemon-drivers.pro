TEMPLATE = app
CONFIG += console c++17
CONFIG -= app_bundle
CONFIG -= qt

QMAKE_CXXFLAGS += -std=c++11 -pthread
LIBS += -pthread

SOURCES += \
        BME680-driver/bme68x.c \
        BME680-driver/common.c \
        SHTC3-driver/shtc3.cpp \
        STC31-driver/stc31.cpp \
        Sensirion-driver-base/sensirion_common.c \
        Sensirion-driver-base/sensirion_driver.cpp \
        drivererror.cpp \
        main.cpp \
        measuremodule.cpp \
        sensormeasure.cpp

HEADERS += \
    BME680-driver/bme68x.h \
    BME680-driver/bme68x_defs.h \
    BME680-driver/common.h \
    SHTC3-driver/shtc3.h \
    STC31-driver/stc31.h \
    Sensirion-driver-base/sensirion_common.h \
    Sensirion-driver-base/sensirion_config.h \
    Sensirion-driver-base/sensirion_driver.h \
    drivererror.h \
    measuremodule.h \
    sensormeasure.h
