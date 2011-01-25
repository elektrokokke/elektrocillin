#include "midiclientwidget.h"
#include "midi2signalclient.h"
#include "jackclient.h"

MidiClientWidget::MidiClientWidget(JackClient *client, const QString &midiInputPortName_, QWidget *parent) :
    QWidget(parent),
    guiClient(new Midi2SignalClient(client->getClientName() + "_gui", this)),
    actualClient(client),
    midiInputPortName(midiInputPortName_)
{
}

MidiClientWidget::~MidiClientWidget()
{
    // make sure that both clients are closed before deleting them:
    guiClient->close();
    actualClient->close();
    delete guiClient;
    delete actualClient;
}

void MidiClientWidget::activate()
{
    // activate both Jack clients:
    if (guiClient->activate() && actualClient->activate()) {
        // connect their MIDI ports:
        guiClient->connectPorts(guiClient->getClientName() + ":" + guiClient->getMidiOutputPortName(), actualClient->getClientName() + ":" + midiInputPortName);
    }
}

void MidiClientWidget::close()
{
    // close both Jack clients:
    guiClient->close();
    actualClient->close();
}
