#ifndef UDPSOCKETHANDLER_H
#define UDPSOCKETHANDLER_H

#include <QObject>
#include <QUdpSocket>
#include <QHostAddress>
#include <QDebug>
#include <QDataStream>

class UdpSocketHandler : public QObject {
    Q_OBJECT

public:
    explicit UdpSocketHandler(QObject *parent = nullptr);
    void BtHsendMessage(const int16_t cmd, const char* data);

private:
    QUdpSocket *m_pUdpSocket;
    QHostAddress m_hostAddress;  // Host PC IP address
    quint16 m_hostPort;  // Port number
};

#endif // UDPSOCKETHANDLER_H
