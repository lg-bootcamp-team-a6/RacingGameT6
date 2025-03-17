#include "UdpSocketHandler.h"

UdpSocketHandler::UdpSocketHandler(QObject *parent) : QObject(parent),m_gameScene(gameScene) {
    m_pUdpSocket = new QUdpSocket(this);

    if (!m_pUdpSocket->bind(QHostAddress::Any, SERVER_PORT)) {
        qDebug() << "Failed to bind UDP socket on port" << SERVER_PORT;
    }
    m_hostPort = SERVER_PORT;
    m_hostAddress = QHostAddress("192.168.10.2");  // Host PC IP address
     connect(m_pUdpSocket, SIGNAL(readyRead()), this, SLOT(processPendingDatagrams()));
}

void UdpSocketHandler::BtHsendMessage(const int16_t cmd, const char* data)
{
     QByteArray datagram;
        QDataStream stream(&datagram, QIODevice::WriteOnly);
        stream.setByteOrder(QDataStream::LittleEndian);

        // 메시지 구성
        stream << cmd;
        stream.writeRawData(static_cast<const char*>(data), sizeof(data));

        qint64 bytesWritten = m_pUdpSocket->writeDatagram(datagram, m_hostAddress, m_hostPort);

        if (bytesWritten == datagram.size()) {
            qDebug() << "---------------Message successfully sent. Bytes written:" << bytesWritten << "Expected:" << datagram.size();
        } else {
            qDebug() << "---------------Error sending message. Bytes written:" << bytesWritten << "Expected:" << datagram.size();
        }
}

void UdpSocketHandler::processPendingDatagrams()
{
    while (m_pUdpSocket->hasPendingDatagrams()) {
        QByteArray datagram;
        datagram.resize(m_pUdpSocket->pendingDatagramSize());
        QHostAddress sender;
        quint16 senderPort;
        m_pUdpSocket->readDatagram(datagram.data(), datagram.size(), &sender, &senderPort);

        // startDoublePlayer()에서 보내는 메시지는 단순 텍스트 "start"입니다.
        QString receivedMessage = QString::fromUtf8(datagram);
        qDebug() << "Received message:" << receivedMessage << "from" << sender.toString()
                 << "port:" << senderPort;

        // 만약 받은 메시지가 "start"라면 추가 처리를 여기에 작성할 수 있습니다.
        if (receivedMessage.trimmed() == "START") {
            qDebug() << "Start command received from UDP server.";
        }
    }
}

