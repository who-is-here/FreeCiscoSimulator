#include "PC.h"
#include "deviceport.h"

#include "ipcommand.h"

PC::PC(const QString &name, const unsigned int &count) : Device (name)
{
    type = dev_PC;
    SetOSType();
    CreatePorts(count);
    InitializeInterfaces(count);
}

void PC::CreatePorts(const quint8 &count)
{
    devicePort *port;
    ports.clear();
    for(quint8 i=1;i<=count;i++)
    {
        port = new devicePort;
        port->number = i;
        port->name = QString("Ethernet%1").arg(i-1);
        port->busy = false;
        ports.insert(QString("Ethernet%1").arg(i-1), port);
    }
    foreach (QString key, ports.keys())
    {
        connect(this, SIGNAL(doSend(quint8,packet&)), ports[key], SLOT(doSend(quint8,packet&)));
        connect(ports[key], SIGNAL(doRecieve(packet&,quint8)), this, SLOT(PacketRecieved(packet&,quint8)));
    }
}

void PC::InitializeInterfaces(const quint8 &count)
{
    deviceInterface link;
    mInterfaces.clear();
    for(quint8 i=1;i<=count;i++)
    {
        foreach (QString key, ports.keys())
            if (ports[key]->number == i)
                link.AssignedPort = ports[key];
        link.name = QString("eth%1").arg(i-1);
        link.mtu = 1500;
        link.state = true;
        extern QString lastMACAddressOctet;
        bool ok;
        int buf = lastMACAddressOctet.toInt(&ok, 16);
        buf += 1;
        if (ok)
        {
            QString str = QString::number(buf, 16);
            if (str.length() == 1)
                link.hwaddr = QString("00:00:00:11:22:0").append(str);
            else if (str.length() == 1)
                link.hwaddr = QString("00:00:00:11:22:").append(str);
            lastMACAddressOctet = str;
        }
        link.address.setIp(QHostAddress(QString("192.168.0.%1").arg(i)));
        link.address.setPrefixLength(24);
        link.address.setBroadcast(CalculateBroadCast(link.address.ip().toString(),link.address.prefixLength()));
        mInterfaces.insert(QString("eth%1").arg(i-1), link);
        addRoute(link.name, link.address.ip().toString(), link.address.prefixLength());
    }
}

QString PC::SetOSType(const QString &os)
{
    this->OSType = os;
    return this->OSType;
}

bool PC::IsLinux()
{
    if (this->OSType == LINUX)
        return true;
    return false;
}

bool PC::IsWindows()
{
    if (this->OSType == WINDOWS)
        return true;
    return false;
}

void PC::onExecuteCommand(const QString &cmd)
{
    CommandToBeInterrupted = "";

    QString c = cmd.simplified();
    QString command = c.section(" ",0,0);
    QString argsStr = c.section(" ",1);
    QStringList args = c.section(" ",1).split(" ");

    if (command == "ip")
        this->ip(argsStr);
    else if (command == "ifconfig" || command == "route" || command == "ifup" || command == "ifdown")
    {
        emit ConsoleWrite("Use \"ip\"");
        emit CommandReturn();
    }
    else if (command == "ping")
        this->ping(args);
    else if (command == "exit")
    {
        emit DisconnectConsole();
    }
    else
    {
        emit ConsoleWrite("command unknown");
        emit CommandReturn();
    }
}

void PC::ip(QString& args)
{
    Command *cmd = new PCIpCommand(this);
    connect(cmd, SIGNAL(Write(QString)), this, SIGNAL(ConsoleWrite(QString)));
    connect(cmd, SIGNAL(Finished()), this, SIGNAL(CommandReturn()));
    cmd->exec(args);
    delete cmd;
}

void PC::ping(QStringList &dst)
{
    QString str = "";
    if (QString(dst[0]).isEmpty())
        dst.replace(0, "--help");
    if (dst[0] == "-h" || dst[0] == "--help")
        str = "Usage: ping [-LRUbdfnqrvVaA] [-c count] [-i interval] [-w deadline]\n"
              "              [-p pattern] [-s packetsize] [-t ttl] [-I interface or address]\n"
              "              [-M mtu discovery hint] [-S sndbuf]\n"
              "              [ -T timestamp option ] [ -Q tos ] [hop1 ...] destination";
    else
    {
        //TODO: Here may be DNS request...
        if (QHostAddress(dst[0]).isNull())
        {
            str = QString("unknown host ").append(dst[0]);
            emit ConsoleWrite(str);
            emit CommandReturn();
        }
        else
        {
            str = QString("PING ").append(dst[0]).append(" (").append(
                        dst[0]).append(") %1(84) bytes of data.").arg(65 /*ttl*/);
            emit ConsoleWrite(str);
            CreateIcmpPacket(dst[0], 8);
        }
    }
}
