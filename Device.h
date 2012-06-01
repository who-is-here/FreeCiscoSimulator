#ifndef DEVICE_H
#define DEVICE_H

#include <QStringList>
#include <QNetworkAddressEntry>
#include <QTcpServer>
class devicePort;

class QIcmpTimer: public QObject
{
    Q_OBJECT
public:
    QIcmpTimer(const QString &dst);
private slots:
    void timerShot();
signals:
    void send(const QString& dst, const quint8& iType);
private:
    QString mDst;
};

struct packet{
    enum protocols {icmp, tcp, udp, igmp};

    protocols proto;
    QHostAddress src;
    QHostAddress dst;
    quint16 sport;
    quint16 dport;
    QString src_mac;
    QString dst_mac;
    QHostAddress nhop;
    quint8 ttl;
    QString data;
};

class Device : public QObject
{
    friend class DeviceCommand;
    Q_OBJECT
    public:
        enum DeviceType {dev_PC, dev_router, dev_switch};

        struct deviceInfo {
            QString id;
            QString name;
            QStringList ipAddresses;
            QStringList links;
        };
        struct deviceLink {
            Device* firstDevice;
            devicePort* firstPort;
            Device* secondDevice;
            devicePort* secondPort;
        };
        struct deviceInterface{
            QString name;
            QNetworkAddressEntry address;
            QString hwaddr;
            devicePort* AssignedPort;
            bool state;
            quint32 mtu;
            // Extend common struct Interface, for ACL of router
            QString in_acl;
            QString out_acl;
        };
        struct route{
            QHostAddress net;
            int prefix;
            QHostAddress gw;
            QString scope;
//            deviceInterface* dev;
            QString dev;
            QString proto;
            QHostAddress src;
            quint8 metric;
        };

        /*struct commands{
            DeviceType type;
            const QString& cmdName;
            cmdPlugin* plg;
        };*/

//        explicit Device ();
//        explicit Device (const QString& );
        explicit Device (const QString& name = "");

        QString SetDeviceID(const QString& id);
        QString SetName(const QString&  name = "DeviceName");
        QString SetDescription(const QString&  desc = "");
        void SetInterfaceState(const QString& name, const bool& state);
        Device::DeviceType GetType() const
            {return type;}
        QString GetName() const
            {return Name;}
        QString GetDeviceID() const
            {return DeviceID;}
        QString GetDescription() const
            {return Description;}
        Device::deviceInfo GetInfo();
        QHash<QString, devicePort*> Ports() const
            { return ports; }

        void AddLink(deviceLink* link);
        QList<deviceLink*> DeleteLink(Device* dev);
        void DeleteLink (Device::deviceLink* link, const bool &del);
        void DeleteLinks();
        static QHostAddress CalculateBroadCast(const QString& addr, const int& pref);
        static QHostAddress CalculateSubNet(const QString& addr, const int& pref);
        void SetupInterface(const QString& name, const QString& addr, const quint8& prefix, const QString& mac, const quint8& port);
        void EraseInterfaces()
            { mInterfaces.clear(); links.clear(); }

        void addRoute (const QString& dev, const QString& addr, const int& prefix=32, QString gw="");
        void delRoute (const QString& dev, const QString& addr, const int& prefix, const QString& gw="");

        QHash<QString, Device::deviceInterface> Interfaces() const
            { return mInterfaces; }
        QList<Device::route> routes() const
            { return mRoutes; }
        QList<Device::deviceLink*> Links() const
            { return links; }

        // Bad idea, but I have no time
        QString cur_cmd;
        QString config_if;
        bool isNegated;
        // End of bad idea


//        typedef void (* cmdPlugin)(const QString& cmd);


    protected:
        virtual void CreatePorts(const quint8& count) = 0;
        virtual void InitializeInterfaces(const quint8& count) = 0;

        DeviceType type;
        QString Name;
        QString Description;
        QString DeviceID;
        QList<Device::deviceLink*> links;
        QHash<QString, devicePort*> ports;
        QList<Device::route> mRoutes;
        QHash<QString, Device::deviceInterface> mInterfaces;

        void ProcessPacket(packet& pkt);
        void RoutePacket(packet& pkt);
        void SendPacket(const packet& pkt, const quint8& port);
        QHostAddress keeped_dst;

        QString CommandToBeInterrupted;

//        QHash<QString, cmdPlugin> commands;
    public slots:
        virtual void onExecuteCommand(const QString& cmd) = 0;
        void PacketRecieved(packet &pkt, const quint8& port);
        void CreateIcmpPacket(const QString& dst, const quint8& iType);
        void onCommandInterrupt(const QString& cmd);
//        void onNewConnection();
    signals:
        void KeepCommandRequest();
        void CommandReturn();
        void ConsoleWrite(const QString& text);
        void ConsoleWriteHTML(const QString& text);
        void ChangePrompt(const QString& newPrompt);
        void PropertiesChnaged();
        void doSend(const quint8& num, packet& pkt);
        void DisconnectConsole();
};

#endif // DEVICE_H
