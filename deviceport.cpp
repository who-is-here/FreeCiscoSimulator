#include "deviceport.h"

devicePort::devicePort(QObject *parent) :
    QObject(parent)
{
}

void devicePort::RecivePacket(ether_frame& frm)
{
    emit doRecieve(frm, number);
}

void devicePort::doSend(const quint8& num, ether_frame& frm)
{
    if (num == number)
    {
        emit SendPacket(frm);
    }
}
