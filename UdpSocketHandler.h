#ifndef UDPSOCKETHANDLER_H
#define UDPSOCKETHANDLER_H

#include <QObject>
#include <QUdpSocket>
#include <QHostAddress>
#include <QDebug>
#include "UdpSocketHandler.h"

class UdpSocketHandler : public QObject {
    Q_OBJECT

public:
    explicit UdpSocketHandler(QObject *parent = nullptr);
    void sendMessage(const QString &message, const QHostAddress &hostAddress, quint16 hostPort);

private:
    QUdpSocket *udpSocket;
    UdpSocketHandler *m_pUdpSocketHandler;
};

#endif // UDPSOCKETHANDLER_H