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
    distortionfilter.cpp \
    midi2signalclient.cpp \
    midicontrollerslider.cpp \
    midicontrollercheckbox.cpp \
    midisplitter.cpp \
    graphview.cpp \
    floattablemodel.cpp \
    jackaudiomodel.cpp \
    record2memoryclient.cpp \
    midipitchslider.cpp \
    graphicsnodeitem.cpp \
    graphicsloglineitem.cpp \
    graphicslineitem.cpp \
    zplanefilter.cpp \
    polynomial.cpp \
    zplanewidget.cpp \
    midicontroller2audioclient.cpp \
    butterworth2polefilter.cpp

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
    distortionfilter.h \
    midi2signalclient.h \
    midicontrollerslider.h \
    midicontrollercheckbox.h \
    midisplitter.h \
    graphview.h \
    floattablemodel.h \
    jackaudiomodel.h \
    record2memoryclient.h \
    midipitchslider.h \
    graphicsnodeitem.h \
    graphicsloglineitem.h \
    graphicslineitem.h \
    zplanefilter.h \
    polynomial.h \
    zplanewidget.h \
    midicontroller2audioclient.h \
    butterworth2polefilter.h

FORMS    += mainwindow.ui \
    zplanewidget.ui

win32:INCLUDEPATH += "C:\Program Files\Jack v1.9.6\includes"
win32:LIBS += $$quote(c:\Program Files\Jack v1.9.6\lib\libjack.a) $$quote(c:\Program Files\Jack v1.9.6\lib\libjackserver.a)
unix:LIBS += -ljack
