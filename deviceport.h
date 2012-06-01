#ifndef DEVICEPORT_H
#define DEVICEPORT_H

#include <QObject>
//class Device;
//struct packet;
#include "Device.h"

class devicePort : public QObject
{
Q_OBJECT
public:
    explicit devicePort(QObject *parent = 0);
    QString name;
    quint8 number;
    bool busy;
signals:
    void SendPacket(packet& pkt);
    void doRecieve (packet& pkt, const quint8& port);
public slots:
    void RecivePacket(packet& pkt);
    void doSend(const quint8& num, packet& pkt);
};
#endif // DEVICEPORT_H
