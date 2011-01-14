#-------------------------------------------------
#
# Project created by QtCreator 2011-01-13T01:41:47
#
#-------------------------------------------------

QT       += core gui

TARGET = elektrocillin
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    midi2audioclient.cpp \
    jackclient.cpp \
    oscillator.cpp \
    simplemonophonicclient.cpp \
    monophonicsynthesizer.cpp \
    pulseoscillator.cpp \
    simplelowpassfilter.cpp \
    adsrenvelope.cpp \
    audiosource.cpp \
    piecewiselinearoscillator.cpp \
    sawtoothoscillator.cpp \
    morphingpiecewiselinearoscillator.cpp \
    distortionfilter.cpp

HEADERS  += mainwindow.h \
    midi2audioclient.h \
    jackclient.h \
    oscillator.h \
    simplemonophonicclient.h \
    monophonicsynthesizer.h \
    pulseoscillator.h \
    simplelowpassfilter.h \
    adsrenvelope.h \
    audiosource.h \
    piecewiselinearoscillator.h \
    sawtoothoscillator.h \
    morphingpiecewiselinearoscillator.h \
    distortionfilter.h

FORMS    += mainwindow.ui

win32:INCLUDEPATH += "C:\Program Files\Jack v1.9.6\includes"
win32:LIBS += $$quote(c:\Program Files\Jack v1.9.6\lib\libjack.a)
