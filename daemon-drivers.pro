TEMPLATE = app
CONFIG += console c++17
CONFIG -= app_bundle
CONFIG -= qt

QMAKE_CXXFLAGS += -std=c++11 -pthread
LIBS += -pthread

SOURCES += \
        BME680-driver/bme68x.c \
        BME680-driver/common.c \
        STC31-driver/sensirion_common.c \
        STC31-driver/sensirion_i2c.c \
        STC31-driver/sensirion_i2c_hal.c \
        STC31-driver/stc3x_i2c.c \
        drivererror.cpp \
        main.cpp \
        measuremodule.cpp \
        sensormeasure.cpp

HEADERS += \
    BME680-driver/bme68x.h \
    BME680-driver/bme68x_defs.h \
    BME680-driver/common.h \
    STC31-driver/sensirion_common.h \
    STC31-driver/sensirion_config.h \
    STC31-driver/sensirion_i2c.h \
    STC31-driver/sensirion_i2c_hal.h \
    STC31-driver/stc3x_i2c.h \
    drivererror.h \
    measuremodule.h \
    sema.h \
    sensormeasure.h
