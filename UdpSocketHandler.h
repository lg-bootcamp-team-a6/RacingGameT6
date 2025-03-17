#ifndef UDPSOCKETHANDLER_H
#define UDPSOCKETHANDLER_H

#include <QObject>
#include <QUdpSocket>
#include <QHostAddress>
#include <QDebug>
#include <QDataStream>
#include "udp/define.h"

struct receive_packet
{
    int cmd;
    char* data;
};

class UdpSocketHandler : public QObject {
    Q_OBJECT

public:
    explicit UdpSocketHandler(QObject *parent = nullptr);
    void BtHsendMessage(const int16_t cmd, const char* data);
    // blocking 방식으로 메시지를 기다리다가 도착하면 파싱해서 리턴하는 함수
    receive_packet receiveMessage();

private:
    QUdpSocket *m_pUdpSocket;
    QHostAddress m_hostAddress;  // Host PC IP address
    quint16 m_hostPort;          // Port number
};

#endif // UDPSOCKETHANDLER_H
