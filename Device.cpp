#include "Device.h"
#include "deviceport.h"
#include "global_variables.h"

#include <QNetworkAddressEntry>
#include <QTimer>

/*Device::Device ()
{
    this->SetName();
    this->InitializeInterfaces(1);
}

Device::Device (const QString &name)
{
    this->SetName(name);
    this->InitializeInterfaces(1);
}*/

Device::Device (const QString &name)
{
    extern QString lastDeviceID;
    lastDeviceID = this->SetDeviceID(lastDeviceID);
    links.clear();
    mRoutes.clear();
    this->SetName(name);
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

void Device::PacketRecieved(packet &pkt, const quint8& port)
{
    qDebug() << "Recieved by " << DeviceID;

    if (pkt.dst == QHostAddress::LocalHost)
    {
        ProcessPacket(pkt);
        return;
    }
    foreach (QString key, mInterfaces.keys())
    {
        if (pkt.dst == mInterfaces[key].address.ip() && port == mInterfaces[key].AssignedPort->number)
        {
            qDebug() << "dst is " << pkt.dst.toString() << "; src is " << pkt.src.toString();
            if (type != dev_PC)
            {
                keeped_dst = pkt.dst;
            }
            ProcessPacket(pkt);
            return;
        }
    }
    if (type != dev_PC)
    {
        qDebug() << "Packet for " << pkt.dst.toString() << " via " << pkt.nhop.toString();
        foreach (deviceInterface iface, mInterfaces)
        {
            if (pkt.nhop == iface.address.ip() && port == iface.AssignedPort->number)
            {
                pkt.nhop = QHostAddress::Any;
                RoutePacket(pkt);
                break;
            }
        }
    }
}

void Device::ProcessPacket(packet &pkt)
{
    if (pkt.proto == packet::icmp)
    {
        if (pkt.data == "echo reply")
        {
            qDebug() << "Inetr = " << CommandToBeInterrupted;
            if (QString("ping ").append(pkt.src.toString()) == CommandToBeInterrupted)
            {
                CommandToBeInterrupted = "";
                return;
            }
            emit ConsoleWrite(QString("Packet received from ").append(pkt.src.toString()));
            QIcmpTimer *timer = new QIcmpTimer(pkt.src.toString());
            connect(timer, SIGNAL(send(QString,quint8)), this, SLOT(CreateIcmpPacket(QString,quint8)));
        }
        else if (pkt.data == "echo request")
        {
            CreateIcmpPacket(pkt.src.toString(), 0);
        }
    }
}

void Device::RoutePacket(packet &pkt)
{
    if (pkt.dst == QHostAddress::LocalHost)
    {
        pkt.src = QHostAddress::LocalHost;
        pkt.src_mac = "00:00:00:00:00:00";
        PacketRecieved(pkt, 0);
        return;
    }
    foreach (const Device::route& r, mRoutes)
    {
        if (pkt.dst.isInSubnet(r.net,r.prefix))
        {
            if (type == dev_router && pkt.nhop != QHostAddress::Any && !keeped_dst.isNull())
            {
                pkt.src = keeped_dst;
                keeped_dst = QHostAddress::Null;
            }
            else if (pkt.nhop == QHostAddress::Any && type == dev_router)
            {
            }
            else
            {
                pkt.src = r.src;
            }
            // Choose interface and port to send packet
            foreach (QString key, mInterfaces.keys())
            {
                if (mInterfaces[key].state == false)
                    continue;
                if (mInterfaces[key].address.ip() == pkt.dst)
                {
                    PacketRecieved(pkt, mInterfaces[key].AssignedPort->number);
                    return;
                }
                else if (mInterfaces[key].address.ip() == r.src)
                {
                    pkt.nhop = r.gw;
                    pkt.src_mac = mInterfaces[key].hwaddr;
                    emit doSend(mInterfaces[key].AssignedPort->number, pkt);
                    return;
                }
            }
        }
    }
}

void Device::CreateIcmpPacket(const QString &dst, const quint8 &iType)
{
    packet pkt;
//            pkt.sport = qrand() % (65535 - 1025) + 1025;
    pkt.proto = packet::icmp;
    pkt.sport = 0;
    pkt.dport = 0;
    pkt.dst = QHostAddress(dst);
    switch(iType)
    {
        case 0:
                pkt.data = "echo reply";
                break;
        case 8:
                pkt.data = "echo request";
                break;
    }
    pkt.ttl = 65;
    RoutePacket(pkt);
}


QIcmpTimer::QIcmpTimer(const QString &dst):
    mDst(dst)
{
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
