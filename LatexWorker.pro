#-------------------------------------------------
#
# Project created by QtCreator 2018-09-11T10:40:11
#
#-------------------------------------------------

QT      += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = latexWSApp
TEMPLATE = app
VERSION = 0.6

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

#DEFINES += DEF_DEBUG_VIEW_MODIFIED_IMAGE

#DEFINES += TEST_DECRYPT_QR_FROM_FILE_BASE64
#DEFINES += TEST_VIEW_QR_PARTS_BASE64

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++14
CONFIG += link_pkgconfig
PKGCONFIG += opencv

SOURCES += \
        main.cpp \
        mainwindow.cpp \
    latexworker.cpp \
    qcompressor.cpp \
    qcrypter.cpp \
    qqrcodereader.cpp

HEADERS += \
        mainwindow.h \
    latexworker.h \
    qcompressor.h \
    qcrypter.h \
    qqrcodereader.h

FORMS += \
        mainwindow.ui

unix {
    LIBS += -lcryptopp
    LIBS += -lz
    LIBS += -lssl
    LIBS += -lzbar
#    LIBS += -L"$$PWD/zxing" -lQZXing
}
win32 {
    LIBS += -L"$$PWD/crypto++/lib/" -lcryptopp
    LIBS += -l ws2_32
    INCLUDEPATH += "$$PWD/crypto++/include"
}

#CONFIG += qzxing_multimedia
#INCLUDEPATH += "$$PWD/zxing"

QMAKE_CXXFLAGS += -std=c++14

unix {
    QMAKE_LFLAGS += -no-pie
    static {
        #message("static=true")
        QMAKE_LFLAGS += -static -static-libgcc -static-libstdc++
        QMAKE_LFLAGS += -Wl,--disable-new-dtags
    }
}

RESOURCES += \
    res.qrc
