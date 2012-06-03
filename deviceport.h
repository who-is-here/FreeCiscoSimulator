#ifndef DEVICEPORT_H
#define DEVICEPORT_H

#include <QObject>
#include "packet.h"

class devicePort : public QObject
{
Q_OBJECT
public:
    explicit devicePort(QObject *parent = 0);
    QString name;
    quint8 number;
    bool busy;
signals:
    void SendPacket(ether_frame& frm);
    void doRecieve (ether_frame& frm, const quint8& port);
public slots:
    void RecivePacket(ether_frame& frm);
    void doSend(const quint8& num, ether_frame& frm);
};
#endif // DEVICEPORT_H
