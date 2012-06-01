#include "deviceport.h"

#include <QApplication>
//#include "Device.h"

devicePort::devicePort(QObject *parent) :
    QObject(parent)
{
}

void devicePort::RecivePacket(packet &pkt)
{
    qDebug() << name;
    emit doRecieve(pkt, number);
}

void devicePort::doSend(const quint8& num, packet& pkt)
{
    if (num == number)
    {
        emit SendPacket(pkt);
    }
}
