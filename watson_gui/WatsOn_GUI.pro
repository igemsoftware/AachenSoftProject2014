#-------------------------------------------------
#
# GUI for WatsOn measurement device
# iGEM Aachen 2014
#
#-------------------------------------------------

QT       += core gui network testlib

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = WatsOn_GUI
TEMPLATE = app

SOURCES += main.cpp\
        igem_mainwindow.cpp \
    imageproc/igem_region.cpp \
    imageproc/igem_srm.cpp \
    imageproc/igem_srm_pixelregion.cpp \
    imageproc/igem_gaussiansmoother.cpp \
    imageproc/igem_hsvmask.cpp \
    imageproc/igem_autoclassify.cpp \
    igem_imageanalyzer.cpp

HEADERS  += igem_mainwindow.h \
    imageproc/igem_region.h \
    imageproc/igem_srm.h \
    imageproc/igem_srm_pixelregion.h \
    imageproc/igem_gaussiansmoother.h \
    imageproc/igem_hsvmask.h \
    imageproc/igem_autoclassify.h \
    igem_imageanalyzer.h

FORMS    +=

DEFINES += QT_NO_WARNING_OUTPUT QT_NO_DEBUG_OUTPUT

INCLUDEPATH += $$PWD/imageproc

CONFIG += warn_off
