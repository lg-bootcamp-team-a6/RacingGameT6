#ifndef UDPRECEIVERWORKER_H
#define UDPRECEIVERWORKER_H
#pragma once
#include <QObject>
#include <QUdpSocket>
#include <QHostAddress>
#include <QDebug>
#include <cstring>
#include "udp/define.h"

// 패킷 구조체 정의
struct receive_packet {
    int cmd;
    char* data;
};

// Qt 메타 타입 시스템에 등록 (헤더에 선언)
Q_DECLARE_METATYPE(receive_packet)

class UdpReceiverWorker : public QObject {
    Q_OBJECT
public:
    explicit UdpReceiverWorker(quint16 port, QObject *parent = nullptr)
        : QObject(parent), m_port(port), m_udpSocket(nullptr) {}

public slots:
    // 이 함수는 worker 스레드에서 실행되며, 여기서 QUdpSocket을 생성합니다.
    void process() {
        // 반드시 이 함수가 실행되는 스레드(즉, worker 스레드)에서 소켓을 생성해야 함
        m_udpSocket = new QUdpSocket();
        if (!m_udpSocket->bind(QHostAddress::Any, m_port)) {
            qWarning() << "Failed to bind UDP socket on port" << m_port;
            return;
        }
        qDebug() << "UdpReceiverWorker: UDP socket bound on port" << m_port;

        qDebug() << "Waiting for incoming datagram...";
        while (true) {
            // 무한 블로킹 대기 (-1: 무한 대기)
            if (m_udpSocket->waitForReadyRead(-1)) {
                while (m_udpSocket->hasPendingDatagrams()) {
                    receive_packet packet;
                    packet.cmd = 0;
                    packet.data = nullptr;

                    QByteArray datagram;
                    datagram.resize(m_udpSocket->pendingDatagramSize());

                    QHostAddress sender;
                    quint16 senderPort;
                    m_udpSocket->readDatagram(datagram.data(), datagram.size(), &sender, &senderPort);

                    qDebug() << "UdpReceiverWorker: Received datagram from" << sender.toString() << "port:" << senderPort;

                    // 최소한 int 크기인지 확인
                    if (datagram.size() < static_cast<int>(sizeof(int))) {
                        qWarning() << "Received datagram too small for parsing.";
                        continue;
                    }

                    // 첫 4바이트를 cmd로 파싱 (필요 시 ntohl() 고려)
                    memcpy(&packet.cmd, datagram.data(), sizeof(int));

                    // 나머지 데이터를 문자열로 파싱
                    int dataSize = datagram.size() - sizeof(int);
                    if (dataSize > 0) {
                        packet.data = new char[dataSize + 1];
                        memcpy(packet.data, datagram.data() + sizeof(int), dataSize);
                        packet.data[dataSize] = '\0';
                    }

                    qDebug() << "UdpReceiverWorker: Parsed packet: cmd =" << packet.cmd
                             << ", data =" << (packet.data ? packet.data : "null");

                    emit packetReceived(packet);
                }
            }
        }
    }

signals:
    void packetReceived(const receive_packet &pkt);

private:
    quint16 m_port;
    QUdpSocket *m_udpSocket;
};

#endif // UDPRECEIVERWORKER_H
