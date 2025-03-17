#ifndef UDPSOCKETHANDLER_H
#define UDPSOCKETHANDLER_H

#include <QObject>
#include <QUdpSocket>
#include <QHostAddress>
#include <QDebug>
#include <QDataStream>
#include "udp/define.h"
#include "gamescene.h"

class UdpSocketHandler : public QObject {
    Q_OBJECT

public:
    explicit UdpSocketHandler(QObject *parent = nullptr);
    void BtHsendMessage(const int16_t cmd, const char* data);


private slots:
    void processPendingDatagrams();

private:
    QUdpSocket *m_pUdpSocket;
    QHostAddress m_hostAddress;  // Host PC IP address
    quint16 m_hostPort;  // Port number
    GameScene *m_gameScene;
};

#endif // UDPSOCKETHANDLER_H
