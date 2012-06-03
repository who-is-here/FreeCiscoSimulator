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
    QIcmpTimer(const quint32& dst);
private slots:
    void timerShot();
signals:
    void send(const quint32& dst, const quint8& iType);
private:
    quint32 mDst;
};

#include "packet.h"

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

        bool allow_forward;

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

        void ProcessIcmpPacket(packet* pkt);
        void RoutePacket(packet* pkt_raw);
        void SendPacket(const ether_frame& frm, const quint8& port);
        QHostAddress keeped_dst;
        bool isTransit;

        QString CommandToBeInterrupted;

    public slots:
        virtual void onExecuteCommand(const QString& cmd) = 0;
        void PacketRecieved(ether_frame& frm, const quint8& port);
        void CreateIcmpPacket(const quint32& dst, const quint8& iType, const quint32& src=1);
        void onCommandInterrupt(const QString& cmd);

    signals:
        void KeepCommandRequest();
        void CommandReturn();
        void ConsoleWrite(const QString& text);
        void ConsoleWriteHTML(const QString& text);
        void ChangePrompt(const QString& newPrompt);
        void PropertiesChnaged();
        void doSend(const quint8& num, ether_frame& frm);
        void DisconnectConsole();
};

#endif // DEVICE_H
