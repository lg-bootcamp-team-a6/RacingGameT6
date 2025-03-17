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
    void process() {
        // 이 스레드에서 QUdpSocket을 생성하고 바인딩합니다.
        m_udpSocket = new QUdpSocket();
        if (!m_udpSocket->bind(QHostAddress::Any, m_port)) {
            qWarning() << "Failed to bind UDP socket on port" << m_port;
            return;
        }
        qDebug() << "UdpReceiverWorker: UDP socket bound on port" << m_port;
        qDebug() << "Waiting for incoming datagram...";

        while (true) {
            // 무한 대기 (-1: 무한 대기)
            if (m_udpSocket->waitForReadyRead(-1)) {
                while (m_udpSocket->hasPendingDatagrams()) {
                    QByteArray datagram;
                    datagram.resize(m_udpSocket->pendingDatagramSize());
                    QHostAddress sender;
                    quint16 senderPort;
                    m_udpSocket->readDatagram(datagram.data(), datagram.size(), &sender, &senderPort);
                    qDebug() << "UdpReceiverWorker: Received datagram from" 
                             << sender.toString() << "port:" << senderPort;
                    
                    // 최소 2바이트(명령) 이상 있어야 함
                    if (datagram.size() < static_cast<int>(sizeof(int16_t))) {
                        qWarning() << "Received datagram too small for parsing.";
                        continue;
                    }
                    
                    receive_packet packet;
                    packet.cmd = 0;
                    packet.data = nullptr;
                    
                    // 첫 2바이트를 명령으로 읽습니다.
                    int16_t cmd_le;
                    memcpy(&cmd_le, datagram.data(), sizeof(cmd_le));
                    int16_t cmd = le16toh(cmd_le);
                    packet.cmd = cmd;
                    
                    // 나머지 바이트를 데이터로 복사합니다.
                    int dataSize = datagram.size() - sizeof(int16_t);
                    if (dataSize > 0) {
                        packet.data = new char[dataSize + 1];
                        memcpy(packet.data, datagram.data() + sizeof(int16_t), dataSize);
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

