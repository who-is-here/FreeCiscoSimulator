#include "Device.h"
#include "deviceport.h"
#include "global_variables.h"

#include <QNetworkAddressEntry>
#include <QTimer>

Device::Device (const QString &name)
{
    extern QString lastDeviceID;
    lastDeviceID = this->SetDeviceID(lastDeviceID);
    links.clear();
    mRoutes.clear();
    this->SetName(name);
    isTransit = false;
}

QString Device::SetDeviceID(const QString &id)
{
    bool ok;
    int buf = id.toInt(&ok, 16);
    buf += 1;
    if (ok)
        this->DeviceID = QString::number(buf, 16);
    else
        this->DeviceID = "null";

    emit PropertiesChnaged();
    return this->DeviceID;
}

QString Device::SetName(const QString &name)
{
    QString pattern = ".+";
    if (QRegExp(pattern, Qt::CaseInsensitive).exactMatch(name))
        this->Name = name;
    else
        this->Name = "DeviceName";
    return this->Name;
}

QString Device::SetDescription(const QString &desc)
{
    this->Description = desc;
    return this->Description;
}

void Device::SetInterfaceState(const QString &name, const bool &state)
{
    foreach (QString key, mInterfaces.keys())
    {
        if (name == key)
        {
            mInterfaces[key].state = state;
        }
    }
}

Device::deviceInfo Device::GetInfo()
{
    deviceInfo info;
    info.id = DeviceID;
    info.name = Name;
    info.ipAddresses.clear();
    foreach (QString key, mInterfaces.keys())
    {
        if (mInterfaces[key].address.ip().toString().isEmpty())
        {
            info.ipAddresses.append(key.append(": "));
        }
        else
        {
            info.ipAddresses.append(key.append(": ").append(mInterfaces[key].address.ip().toString()).append("/%1").arg(
                    mInterfaces[key].address.prefixLength()));
        }
    }
    info.links.clear();
    foreach(deviceLink *link, links)
    {
        info.links.append(link->firstDevice->GetDeviceID().append(":%1").arg(
                              link->firstPort->number).append(":").append(
                              link->secondDevice->DeviceID).append(":").append(QString("%1").arg(link->secondPort->number)));
    }
    return info;
}

void Device::AddLink(deviceLink* link)
{
    links.append(link);
}

void Device::addRoute(const QString &dev, const QString &addr, const int &prefix, QString gw)
{
    // Some checks of data
    bool device_exist = false;
    foreach (QString key, mInterfaces.keys())
    {
        if (key == dev)
        {
            device_exist = true;
            break;
        }
    }
    if (!device_exist)
    {
        return;
    }
    bool isReachable = false;
    foreach (const route& r, mRoutes)
    {
        if (QHostAddress(gw).isInSubnet(r.net,r.prefix))
        {
            isReachable = true;
            break;
        }
    }
    if (!isReachable)
    {
        gw.clear();
    }

    // Create route
    route r;
    r.net = CalculateSubNet(addr, prefix);
    r.prefix = prefix;
    r.dev = dev;
    r.gw = gw;
    foreach (QString key, mInterfaces.keys())
    {
        if (dev != key)
        {
            continue;
        }
        if (QHostAddress(addr) == mInterfaces[key].address.ip() && r.prefix == mInterfaces[key].address.prefixLength())
        {
            r.proto = "kernel";
            r.scope = "link";
            r.src = addr;
            // ***********************
            // until the interface can have only one address
            int idx = 0;
            QMutableListIterator<Device::route> it(mRoutes);
            while (it.hasNext())
            {
                if (it.next().dev == dev)
                {
                    it.remove();
                    break;
                }
                else
                {
                    ++idx;
                }
            }
//            routes.replace(idx,r);
            mRoutes.insert(idx,r);
            break;
            // ***********************
        }
        else
        {
            r.proto = "";
            if (gw.isEmpty())
                r.scope = "link";
            else
                r.scope = "global";
            r.src = mInterfaces[key].address.ip();
            mRoutes.append(r);
            break;
        }
    }
}

void Device::delRoute(const QString &dev, const QString &addr, const int &prefix, const QString &gw)
{
    QMutableListIterator<Device::route> it(mRoutes);
    while (it.hasNext())
        if (it.next().dev == dev && it.value().net == QHostAddress(addr) && it.value().prefix == prefix &&
            (it.value().gw == QHostAddress(gw) || gw == ""))
            it.remove();
}

QList<Device::deviceLink*> Device::DeleteLink(Device* dev)
{
    QList<Device::deviceLink*> _links;
    _links.clear();
    foreach(Device::deviceLink *link, links)
        if (link->secondDevice == dev || link->firstDevice == dev)
            _links.append(link);

    if (_links.count() > 1)
        return _links;

    // If only one link between devices, then remove it
    // end free port
    disconnect(_links.first()->firstPort, SIGNAL(SendPacket(packet&)),  _links.first()->secondPort, SLOT(RecivePacket(packet&)));
    disconnect(_links.first()->secondPort, SIGNAL(SendPacket(packet&)), _links.first()->firstPort, SLOT(RecivePacket(packet&)));
    QMutableListIterator<Device::deviceLink*> it(_links.first()->firstDevice->links);
    while (it.hasNext())
    {
        if (it.next() == _links.first())
        {
            it.remove();
        }
    }

    // clear remote device
    it = _links.first()->secondDevice->links;
    while (it.hasNext())
    {
        if (it.next() == _links.first())
        {
            it.remove();
        }
    }
    _links.first()->firstPort->busy = false;
    _links.first()->secondPort->busy = false;

    delete _links.first();
    _links.removeAt(0);

    return _links;
}

void Device::DeleteLink(Device::deviceLink *link, const bool &del)
{
    disconnect(link->firstPort, SIGNAL(SendPacket(packet&)),  link->secondPort, SLOT(RecivePacket(packet&)));
    disconnect(link->secondPort, SIGNAL(SendPacket(packet&)), link->firstPort, SLOT(RecivePacket(packet&)));
    QMutableListIterator<Device::deviceLink*> it(links);
    while (it.hasNext()) {
        if (it.next() == link)
        {
            it.value()->firstPort->busy = false;
            it.value()->secondPort->busy = false;
            if (del)
            {
                it.value()->secondDevice->DeleteLink(it.value(), false);
                delete it.value();
            }
            it.remove();
        }
    }
}

void Device::DeleteLinks()
{
    QMutableListIterator<Device::deviceLink*> it(links);
    while (it.hasNext())
    {
        it.next();
        it.value()->firstDevice->DeleteLink(it.value(), true);
    }
}

QHostAddress Device::CalculateBroadCast(const QString &addr, const int &pref)
{
    bool ok;
    QString prefix_str = "";
    for (int x=1; x<33; x++)
    {
        if (x > pref)
        {
            prefix_str.append("1");
        }
        else
        {
            prefix_str.append("0");
        }
    }
    quint32 prefix = prefix_str.toLongLong(&ok,2);
    quint32 broadcast = QHostAddress(addr).toIPv4Address() | prefix;
    return QHostAddress(broadcast);
}

QHostAddress Device::CalculateSubNet(const QString &addr, const int &pref)
{
    bool ok;
    QString prefix_str = "";
    for (int x=1; x<33; x++)
    {
        if (x > pref)
        {
            prefix_str.append("1");
        }
        else
        {
            prefix_str.append("0");
        }
    }
    quint32 prefix = prefix_str.toLongLong(&ok,2);
    quint32 broadcast = QHostAddress(addr).toIPv4Address() | prefix;
    return QHostAddress(broadcast-prefix);
}

void Device::PacketRecieved(ether_frame &frm, const quint8 &port)
{
    foreach (deviceInterface iface, mInterfaces)
    {
        // If frame for this device
        if (iface.hwaddr == frm.dst_mac && iface.AssignedPort->number == port || port == 0)
        {
            // If packet for this device process it else try to route
            if (frm.pkt->dstip == iface.address.ip().toIPv4Address() ||
                QHostAddress(frm.pkt->dstip) == QHostAddress::LocalHost)
            {
                if (frm.pkt->type == "ip")
                {
                    // processing ip packet
                }
                else if (frm.pkt->type == "icmp")
                {
                    isTransit = false;
                    ProcessIcmpPacket(frm.pkt);
                }
            }
            else
            {
                if (allow_forward)
                {
                    isTransit = true;
                    RoutePacket(frm.pkt);
                }
            }
            break;
        }
    }
}

void Device::ProcessIcmpPacket(packet* pkt)
{
    icmp_packet* ipkt = static_cast<icmp_packet*>(pkt);
    if (ipkt->icmp_type == 0)
    {
        if (QString("ping ").append(QHostAddress(ipkt->srcip).toString()) == CommandToBeInterrupted)
        {
            CommandToBeInterrupted = "";
            return;
        }
        emit ConsoleWrite(QString("Packet received from ").append(QHostAddress(ipkt->srcip).toString()));
        QIcmpTimer* timer = new QIcmpTimer(ipkt->srcip);
        connect(timer, SIGNAL(send(quint32,quint8)), this, SLOT(CreateIcmpPacket(quint32,quint8)));
    }
    else if (ipkt->icmp_type == 8)
    {
        CreateIcmpPacket(ipkt->srcip, 0, ipkt->dstip);
    }
}

void Device::RoutePacket(packet* pkt)
{
    if (QHostAddress(pkt->dstip) == QHostAddress::LocalHost)
    {
        pkt->srcip = QHostAddress(QHostAddress::LocalHost).toIPv4Address();
        ether_frame frm;
        frm.src_mac = "";
        frm.dst_mac = "";
        frm.pkt = pkt;
        PacketRecieved(frm, 0);
        return;
    }
    foreach (const Device::route& r, mRoutes)
    {
        if (QHostAddress(pkt->dstip).isInSubnet(r.net,r.prefix))
        {
            if (!isTransit && pkt->srcip == 1)
            {
                pkt->srcip = r.src.toIPv4Address();
                isTransit = false;
            }

            // Choose interface and port to send packet
            foreach (deviceInterface iface, mInterfaces)
            {
                if (iface.state == false)
                    continue;
                // Packet for itself don't need be routing, it don't have table=local
                if (pkt->dstip == iface.address.ip().toIPv4Address())
                {
                    ether_frame frm;
                    frm.src_mac = iface.hwaddr;
                    frm.dst_mac = iface.hwaddr;
                    frm.pkt = pkt;
                    PacketRecieved(frm, iface.AssignedPort->number);
                    return;
                }
                // if all OK — incasulate in ethernet frame and route it
                else if (iface.address.ip() == r.src)
                {
                    ether_frame frm;
                    frm.src_mac = iface.hwaddr;
                    frm.dst_mac = "";
                    frm.pkt = pkt;

                    // Arp protocol don't realize by design
                    // Until we haven't switch, I realize this algoritm
                    // CAUTION! It won't be work if we have switch on the end of patch-cord!!!
                    foreach (deviceLink* lnk, links)
                    {
                        Device* d;
                        devicePort* p;
                        if (lnk->firstPort == iface.AssignedPort)
                        {
                            d = lnk->secondDevice;
                            p = lnk->secondPort;
                        }
                        else if (lnk->secondPort == iface.AssignedPort)
                        {
                            d = lnk->firstDevice;
                            p = lnk->firstPort;
                        }
                        else
                        {
                            continue;
                        }
                        foreach (deviceInterface riface, d->Interfaces())
                        {
                            if (riface.AssignedPort == p)
                            {
                                frm.dst_mac = riface.hwaddr;
                            }
                        }
                    }
                    // End of arp fiction!

                    emit doSend(iface.AssignedPort->number, frm);
                    return;
                }
            }
        }
    }
}

void Device::CreateIcmpPacket(const quint32& dst, const quint8& iType, const quint32& src)
{
    icmp_packet* pkt = new icmp_packet();
    pkt->type = "icmp";
    pkt->dstip = dst;
    pkt->srcip = src;
    pkt->icmp_type = iType;
    pkt->ttl = 65;
    RoutePacket(pkt);
}


QIcmpTimer::QIcmpTimer(const quint32& dst)
{
    mDst = dst;
    QTimer::singleShot(1000, this, SLOT(timerShot()));
}

void QIcmpTimer::timerShot()
{
    emit send(mDst, 8);

    deleteLater();
}

void Device::onCommandInterrupt(const QString &cmd)
{
    CommandToBeInterrupted = cmd;
}

void Device::SetupInterface(const QString& name, const QString& addr, const quint8& prefix, const QString& mac, const quint8& port)
{
    deviceInterface iface;

    iface.name = name.simplified();

    iface.address.setIp(QHostAddress(addr.simplified()));

    if (prefix < 33)
    {
        iface.address.setPrefixLength(prefix);
    }

    bool isMac = true;
    bool ok;
    QStringList mac_parts = mac.simplified().split(":");
    foreach (QString mac_part, mac_parts)
    {
        mac_part.toInt(&ok, 16);
        if (!ok)
        {
            isMac = false;
            break;
        }
    }
    if (isMac)
    {
        iface.hwaddr = mac.simplified();
    }
    else
    {
        iface.hwaddr = "11:22:33:44:55:66";
    }

    iface.AssignedPort = NULL;
    foreach (QString key, ports.keys())
    {
        if (ports[key]->number == port)
        {
            iface.AssignedPort = ports[key];
        }
    }

    if (iface.AssignedPort != NULL && !iface.name.isEmpty())
    {
        iface.state = true;
        mInterfaces.insert(iface.name, iface);
        addRoute(iface.name, iface.address.ip().toString(), iface.address.prefixLength());
    }
}
