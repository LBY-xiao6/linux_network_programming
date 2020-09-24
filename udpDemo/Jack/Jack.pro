TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.c \
    ../../../../公共代码/wrap.c

INCLUDEPATH += \
    /mnt/hgfs/codes/公共代码

HEADERS += \
    ../../../../公共代码/common.h
