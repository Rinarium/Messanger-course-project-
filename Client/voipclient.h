#ifndef VOIPCLIENT_H
#define VOIPCLIENT_H

#include <QThread>
#include "audio.h"

class VoIPClient : public QThread
{
    Q_OBJECT
    public:
        explicit VoIPClient(QObject *parent = 0);

        void setLocalPort(quint16 port);
        void setRemoteHost(QHostAddress host);
        void setRemotePort(quint16 port);

    public slots:
        void startCall(void);
        void finishCall(void);
        void disconnectUser(void);

    signals:
        endCall();

    protected:
        void run(void);

    private:
        AudioTransmitter * transmitter;
        AudioReciever * reciever;
        QHostAddress remoteHost;
        quint16 localPort;
        quint16 remotePort;
};

#endif // VOIPCLIENT_H
