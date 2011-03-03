#-------------------------------------------------
#
# Project created by QtCreator 2011-01-13T01:41:47
#
#-------------------------------------------------

QT      += core gui
QT      += opengl

TARGET = elektrocillin
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    midi2audioclient.cpp \
    jackclient.cpp \
    oscillator.cpp \
    monophonicsynthesizer.cpp \
    simplelowpassfilter.cpp \
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
    interpolator.cpp \
    linearinterpolator.cpp \
    cubicsplineinterpolator.cpp \
    graphicsinterpolationitem.cpp \
    linearwaveshapingclient.cpp \
    cubicsplinewaveshapingclient.cpp \
    linearintegralinterpolator.cpp \
    linearoscillator.cpp \
    linearoscillatorclient.cpp \
    linearmorphoscillator.cpp \
    pulseoscillator.cpp \
    multiplyprocessor.cpp \
    metajack/midiport.cpp \
    metajack/metajackclient.cpp \
    metajack/metajackport.cpp \
    metajack/metajackcontext.cpp \
    metajack/metajack.cpp \
    whitenoisegenerator.cpp \
    graphicsinterpolatoredititem.cpp \
    cisi.cpp \
    linearmorphoscillatorclient.cpp \
    jackringbuffer.cpp \
    oscillatorclient.cpp \
    eventprocessorclient.cpp \
    envelope.cpp \
    envelopeclient.cpp \
    graphicsclientnameitem.cpp \
    graphicsportconnectionitem.cpp \
    metajack/realjackcontext.cpp \
    metajack/recursivejackcontext.cpp \
    jackcontextgraphicsscene.cpp \
    wheelzoominggraphicsview.cpp \
    graphicsclientitem.cpp \
    graphicsportitem.cpp

HEADERS  += mainwindow.h \
    midi2audioclient.h \
    jackclient.h \
    oscillator.h \
    monophonicsynthesizer.h \
    simplelowpassfilter.h \
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
    eventprocessorclient.h \
    jackthreadeventprocessorclient.h \
    jacknullclient.h \
    interpolator.h \
    linearinterpolator.h \
    cubicsplineinterpolator.h \
    graphicsinterpolationitem.h \
    linearwaveshapingclient.h \
    cubicsplinewaveshapingclient.h \
    linearintegralinterpolator.h \
    linearoscillator.h \
    linearoscillatorclient.h \
    linearmorphoscillator.h \
    pulseoscillator.h \
    multiplyprocessor.h \
    metajack/midiport.h \
    metajack/metajackclient.h \
    metajack/metajackport.h \
    metajack/metajackcontext.h \
    metajack/callbackhandlers.h \
    whitenoisegenerator.h \
    graphicsinterpolatoredititem.h \
    cisi.h \
    linearmorphoscillatorclient.h \
    oscillatorclient.h \
    envelope.h \
    envelopeclient.h \
    graphicsclientnameitem.h \
    graphicsportconnectionitem.h \
    metajack/realjackcontext.h \
    metajack/metajack.h \
    metajack/recursivejackcontext.h \
    metajack/jackcontext.h \
    jackcontextgraphicsscene.h \
    wheelzoominggraphicsview.h \
    graphicsportitem.h \
    graphicsclientitem.h

FORMS    += mainwindow.ui \
    zplanewidget.ui

win32:INCLUDEPATH += "C:\\Program Files\\Jack v1.9.6\\includes"
win32:INCLUDEPATH += C:\\boost_1_45_0
win32:LIBS += $$quote(c:\\Program Files\\Jack v1.9.6\\lib\\libjack.a) $$quote(c:\\Program Files\\Jack v1.9.6\\lib\\libjackserver.a)
unix:LIBS += -ljack

OTHER_FILES +=
