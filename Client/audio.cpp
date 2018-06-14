#include "audio.h"

QAudioFormat GetStreamAudioFormat(void)
{
    QAudioFormat format;
    format.setSampleRate(44100);
    format.setChannelCount(1);
    format.setSampleSize(16);
    format.setCodec("audio/pcm");
    format.setByteOrder(QAudioFormat::LittleEndian);
    format.setSampleType(QAudioFormat::UnSignedInt);

    QAudioDeviceInfo info = QAudioDeviceInfo::defaultInputDevice();
    if (!info.isFormatSupported(format))
        format = info.nearestFormat(format);

    return format;
}

//////////////////////////////////
// AudioTransmitter Class
//////////////////////////////////

AudioTransmitter::AudioTransmitter(quint16 localPort, QHostAddress remoteHost, quint16 remotePort)
    : maxsize(8192)
{
    this->localPort = localPort;
    this->remoteHost = remoteHost;
    this->remotePort = remotePort;
}

void AudioTransmitter::startCall(void)
{
    audio_input = new QAudioInput(GetStreamAudioFormat());
    audio_device = audio_input->start();
    connect(audio_device, SIGNAL(readyRead()), this, SLOT(sendDatagrams()));
}

void AudioTransmitter::stopCall(void)
{
    audio_input->stop();
    delete audio_input;
    socket.close();
    disconnect(audio_device, SIGNAL(readyRead()), this, SLOT(sendDatagrams()));
}


void AudioTransmitter::sendDatagrams(void)
{
    QByteArray tmp = audio_device->read(maxsize);
    socket.writeDatagram(tmp.data(), tmp.size(), remoteHost, remotePort);
}


//////////////////////////////////
// AudioReciever Class
//////////////////////////////////

AudioReciever::AudioReciever(quint16 localPort, QHostAddress remoteHost, quint16 remotePort)
{
    this->localPort = localPort;
    this->remoteHost = remoteHost;
    this->remotePort = remotePort;
}

bool AudioReciever::startCall(void)
{
    audio_output = new QAudioOutput(GetStreamAudioFormat());
    audio_device = audio_output->start();
    connect(&socket, SIGNAL(readyRead()), this, SLOT(readPendingDatagrams()));
    return socket.bind(QHostAddress::Any, localPort, QUdpSocket::DontShareAddress);
}

void AudioReciever::stopCall(void)
{
    audio_output->stop();
    delete audio_output;
    socket.close();
    disconnect(&socket, SIGNAL(readyRead()), this, SLOT(readPendingDatagrams()));
}

void AudioReciever::readPendingDatagrams(void)
{
    while (socket.hasPendingDatagrams())
    {
        qint64 size = socket.pendingDatagramSize();
        char * data = new char[size];
        socket.readDatagram(data, size);
        audio_device->write(data, size);
    }
}

QUdpSocket* AudioReciever::getSocket()
{
    return &socket;
}

