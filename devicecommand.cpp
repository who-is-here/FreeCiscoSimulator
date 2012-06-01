#include "devicecommand.h"

#include "Device.h"


DeviceCommand::DeviceCommand(const QString &command, Device *device):
    Command(command),
    mDevice(device)
{}

void DeviceCommand::ConfigureInterface(const QString &name, const QString &addr, const int &prefix, quint32 mtu)
{

    foreach(QString key, mDevice->mInterfaces.keys())
    {
        if (mDevice->mInterfaces[key].name == name)
        {
            if (addr.isEmpty())
            {
                delRoute(name, CalculateSubNet(mDevice->mInterfaces[key].address.ip().toString(),
                                               mDevice->mInterfaces[key].address.prefixLength()).toString(),
                         mDevice->mInterfaces[key].address.prefixLength());
            }
            mDevice->mInterfaces[key].address.setIp(QHostAddress(addr));
            mDevice->mInterfaces[key].address.setPrefixLength(prefix);
            mDevice->mInterfaces[key].address.setBroadcast(mDevice->CalculateBroadCast(addr,prefix));
            mDevice->mInterfaces[key].mtu = mtu;
            if (!addr.isEmpty())
                addRoute(name, addr, prefix);
        }
    }

    emit mDevice->PropertiesChnaged();
}

void DeviceCommand::addRoute(const QString &dev, const QString &addr, const int &prefix, const QString &gw)
{
    mDevice->addRoute(dev, addr, prefix, gw);
}

void DeviceCommand::delRoute(const QString &dev, const QString &addr, const int &prefix, const QString &gw)
{
    device()->delRoute(dev, addr, prefix, gw);
}

QHostAddress DeviceCommand::CalculateSubNet(const QString &addr, const int &pref)
{
    return mDevice->CalculateSubNet(addr, pref);
}
