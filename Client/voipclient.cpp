#include "voipclient.h"

VoIPClient::VoIPClient(QObject *parent) : QThread(parent)
{
}

void VoIPClient::run(void)
{
    transmitter = new AudioTransmitter(localPort, remoteHost, remotePort);
    reciever = new AudioReciever(localPort, remoteHost, remotePort);
    connect(reciever->getSocket(), SIGNAL(disconnected()), this, SLOT(disconnectUser()));
    reciever->startCall();
    transmitter->startCall();

    exec();

    transmitter->stopCall();
    reciever->stopCall();
    disconnect(reciever->getSocket(), SIGNAL(disconnected()), this, SLOT(disconnectUser()));
    delete transmitter;
    delete reciever;
}

void VoIPClient::startCall(void)
{
    start();
}

void VoIPClient::finishCall(void)
{
    exit(0);
}

void VoIPClient::setLocalPort(quint16 port)
{
    localPort = port;
}

void VoIPClient::setRemoteHost(QHostAddress host)
{
    remoteHost = host;
}

void VoIPClient::setRemotePort(quint16 port)
{
    remotePort = port;
}

void VoIPClient::disconnectUser(void)
{
    this->quit();
    emit endCall();
}
