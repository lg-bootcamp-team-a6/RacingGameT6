#include "UdpSocketHandler.h"

UdpSocketHandler::UdpSocketHandler(QObject *parent) : QObject(parent) {
    m_pUdpSocket = new QUdpSocket(this);

    m_hostAddress = QHostAddress("192.168.10.2");  // Host PC IP address
    m_hostPort = 12345;  // Port number
}

void UdpSocketHandler::sendMessage(const QString &message) {
    QByteArray datagram = message.toUtf8();
    qint64 bytesWritten = m_pUdpSocket->writeDatagram(datagram, m_hostAddress, m_hostPort);
    if (bytesWritten == datagram.size()) {
        qDebug() << "---------------Message successfully sent:" << message << bytesWritten << "Expected:" << datagram.size();
    } else {
        qDebug() << "---------------Error sending message. Bytes written:" << bytesWritten << "Expected:" << datagram.size();
    }
}
