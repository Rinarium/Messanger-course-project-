#ifndef AUDIO_H
#define AUDIO_H

#include <QObject>
#include <QIODevice>
#include <QUdpSocket>
#include <QHostAddress>
#include <QAudioInput>
#include <QAudioOutput>

// Function returns constant audio format of all audio streams.
QAudioFormat GetStreamAudioFormat(void);

// Class produces a device to capture and send audio over the network.
class AudioTransmitter : public QObject
{
    Q_OBJECT
    public:
        AudioTransmitter(quint16 localPort, QHostAddress remoteHost, quint16 remotePort);
        void startCall(void);
        void stopCall(void);

    private:
        QUdpSocket socket;
        QAudioInput * audio_input;
        QIODevice * audio_device;
        quint16 localPort;
        QHostAddress remoteHost;
        quint16 remotePort;
        const qint64 maxsize;

    private slots:
        void sendDatagrams(void);
};

// Class produces a device to recieve audio from the network and play it.
class AudioReciever : public QObject
{
    Q_OBJECT
    public:
        AudioReciever(quint16 localPort, QHostAddress remoteHost, quint16 remotePort);
        bool startCall(void);
        void stopCall(void);
        QUdpSocket* getSocket();

    private:
        QUdpSocket socket;
        QAudioOutput * audio_output;
        QIODevice * audio_device;
        quint16 localPort;
        QHostAddress remoteHost;
        quint16 remotePort;

    private slots:
        void readPendingDatagrams(void);
};

#endif // AUDIO_H
