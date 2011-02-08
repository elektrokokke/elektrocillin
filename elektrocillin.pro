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
    monophonicsynthesizer.cpp \
    pulseoscillator.cpp \
    simplelowpassfilter.cpp \
    adsrenvelope.cpp \
    piecewiselinearoscillator.cpp \
    sawtoothoscillator.cpp \
    morphingpiecewiselinearoscillator.cpp \
    distortionfilter.cpp \
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
    zplanewidget.cpp \
    midicontroller2audioclient.cpp \
    butterworth2polefilter.cpp \
    jackthread.cpp \
    jackclientwithdeferredprocessing.cpp \
    frequencyresponsegraphicsitem.cpp \
    iirfilter.cpp \
    iirbutterworthfilter.cpp \
    iirmoogfilter.cpp \
    graphicskeyitem.cpp \
    graphicskeyboarditem.cpp \
    iirmoogfilterclient.cpp \
    midisignalclient.cpp \
    audioprocessor.cpp \
    audioprocessorclient.cpp \
    midiprocessor.cpp \
    midiprocessorclient.cpp \
    jacknullclient.cpp \
    graphicsclientitem.cpp \
    interpolator.cpp \
    linearinterpolator.cpp \
    cubicsplineinterpolator.cpp \
    graphicsinterpolationitem.cpp \
    pulseoscillator2.cpp \
    linearwaveshapingclient.cpp \
    cubicsplinewaveshapingclient.cpp \
    visiblerectanglegraphicsview.cpp \
    linearintegralinterpolator.cpp \
    linearoscillator.cpp \
    linearoscillatorclient.cpp

HEADERS  += mainwindow.h \
    midi2audioclient.h \
    jackclient.h \
    oscillator.h \
    monophonicsynthesizer.h \
    pulseoscillator.h \
    simplelowpassfilter.h \
    adsrenvelope.h \
    piecewiselinearoscillator.h \
    sawtoothoscillator.h \
    morphingpiecewiselinearoscillator.h \
    distortionfilter.h \
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
    butterworth2polefilter.h \
    jackringbuffer.h \
    jackthread.h \
    jackclientwithdeferredprocessing.h \
    frequencyresponsegraphicsitem.h \
    iirfilter.h \
    iirbutterworthfilter.h \
    iirmoogfilter.h \
    frequencyresponse.h \
    graphicskeyitem.h \
    graphicskeyboarditem.h \
    iirmoogfilterclient.h \
    midisignalclient.h \
    audioprocessor.h \
    audioprocessorclient.h \
    midiprocessor.h \
    midiprocessorclient.h \
    eventprocessor.h \
    eventprocessorclient.h \
    jackthreadeventprocessorclient.h \
    jacknullclient.h \
    graphicsclientitem.h \
    interpolator.h \
    linearinterpolator.h \
    cubicsplineinterpolator.h \
    graphicsinterpolationitem.h \
    pulseoscillator2.h \
    linearwaveshapingclient.h \
    cubicsplinewaveshapingclient.h \
    visiblerectanglegraphicsview.h \
    linearintegralinterpolator.h \
    linearoscillator.h \
    linearoscillatorclient.h

FORMS    += mainwindow.ui \
    zplanewidget.ui

win32:INCLUDEPATH += "C:\Program Files\Jack v1.9.6\includes"
win32:LIBS += $$quote(c:\Program Files\Jack v1.9.6\lib\libjack.a) $$quote(c:\Program Files\Jack v1.9.6\lib\libjackserver.a)
unix:LIBS += -ljack
