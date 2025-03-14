#include "UdpSocketHandler.h"

UdpSocketHandler::UdpSocketHandler(QObject *parent) : QObject(parent) {
    udpSocket = new QUdpSocket(this);
}

void UdpSocketHandler::sendMessage(const QString &message, const QHostAddress &hostAddress, quint16 hostPort) {
    QHostAddress hostAddress("192.168.10.2");  // Host PC IP address
    quint16 hostPort = 12345;  // Port number
    QByteArray datagram = message.toUtf8();
    qint64 bytesWritten = udpSocket->writeDatagram(datagram, hostAddress, hostPort);

    if (bytesWritten == datagram.size()) {
        qDebug() << "---------------Message successfully sent:" << message << bytesWritten << "Expected:" << datagram.size();
    } else {
        qDebug() << "---------------Error sending message. Bytes written:" << bytesWritten << "Expected:" << datagram.size();
    }
}