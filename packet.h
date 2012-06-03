#ifndef PACKET_H
#define PACKET_H

#include <QString>

class packet
{
public:
    packet(){}
    QString type;
    quint8 ttl;
    // We have only ip and icmp packet. Both of them have src and dst ip address.
    quint32 srcip;
    quint32 dstip;
};

class ip_packet : public packet
{
public:
    ip_packet(){}
    quint16 srcport;
    quint16 dstport;
    QString data;
};

class icmp_packet : public packet
{
public:
    icmp_packet(){}
    quint8 icmp_type;
};

class ether_frame
{
public:
    ether_frame(){}
    QString src_mac;
    QString dst_mac;
    packet* pkt;
};

#endif // PACKET_H
