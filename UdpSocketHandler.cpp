#include "UdpSocketHandler.h"

UdpSocketHandler::UdpSocketHandler(QObject *parent) : QObject(parent){
    m_pUdpSocket = new QUdpSocket(this);

    //if (!m_pUdpSocket->bind(QHostAddress::Any, SERVER_PORT)) {
    //    qDebug() << "Failed to bind UDP socket on port" << SERVER_PORT;
    //}
    m_hostPort = SERVER_PORT;
    m_hostAddress = QHostAddress("192.168.10.2");  // Host PC IP address
     //connect(m_pUdpSocket, SIGNAL(readyRead()), this, SLOT(processPendingDatagrams()));
}

void UdpSocketHandler::BtHsendMessage(const int16_t cmd, const char* data)
{
     QByteArray datagram;
        QDataStream stream(&datagram, QIODevice::WriteOnly);
        stream.setByteOrder(QDataStream::LittleEndian);

        // 메시지 구성
        stream << cmd;
        stream.writeRawData(static_cast<const char*>(data), strlen(data) + 1);

        qint64 bytesWritten = m_pUdpSocket->writeDatagram(datagram, m_hostAddress, m_hostPort);

        if (bytesWritten == datagram.size()) {
            qDebug() << "---------------Message successfully sent. Bytes written:" << bytesWritten << "Expected:" << datagram.size();
        } else {
            qDebug() << "---------------Error sending message. Bytes written:" << bytesWritten << "Expected:" << datagram.size();
        }
}



// receive_packet UdpSocketHandler::receiveMessage()
// {
//     receive_packet packet;
//     packet.cmd = 0;
//     packet.data = nullptr;

//     qDebug() << "Waiting for incoming datagram...";

//     // 무한루프를 돌면서 데이터 도착을 기다림 (blocking 방식)
//     while (true) {
//         // -1: 무한 대기, 데이터가 도착하면 true를 리턴
//         if (m_pUdpSocket->waitForReadyRead(-1)) {
//             // 데이터가 도착했으므로 pending datagram 확인
//             qDebug() << "received!";
//             if (m_pUdpSocket->hasPendingDatagrams()) {
//                 QByteArray datagram;
//                 datagram.resize(m_pUdpSocket->pendingDatagramSize());

//                 QHostAddress sender;
//                 quint16 senderPort;
//                 m_pUdpSocket->readDatagram(datagram.data(), datagram.size(), &sender, &senderPort);

//                 qDebug() << "Received datagram from" << sender.toString() << "port:" << senderPort;

//                 // 최소한 cmd (int) 크기 이상인지 확인
//                 if (datagram.size() < static_cast<int>(sizeof(int))) {
//                     qWarning() << "Received datagram too small for parsing.";
//                     continue; // datagram이 올바르지 않으면 계속 대기
//                 }

//                 // 첫 4바이트를 cmd로 파싱 (네트워크 바이트 순서인 경우 ntohl() 고려)
//                 memcpy(&packet.cmd, datagram.data(), sizeof(int));

//                 // 나머지 데이터를 문자열로 파싱
//                 int dataSize = datagram.size() - sizeof(int);
//                 if (dataSize > 0) {
//                     packet.data = new char[dataSize + 1];
//                     memcpy(packet.data, datagram.data() + sizeof(int), dataSize);
//                     packet.data[dataSize] = '\0';
//                 }

//                 qDebug() << "Parsed packet: cmd =" << packet.cmd
//                          << ", data =" << (packet.data ? packet.data : "null");

//                 break;  // 올바른 패킷을 읽었으므로 루프 종료
//             }
//         }
//     }

//     return packet;
// }