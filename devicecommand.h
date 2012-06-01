#ifndef DEVICECOMMAND_H
#define DEVICECOMMAND_H

#include "command.h"

#include <QHostAddress>

class DeviceCommand: public Command
{
protected:
    DeviceCommand(const QString &command, Device *device);

    Device *device() const
        { return mDevice; }

    void ConfigureInterface (const QString& name, const QString& addr, const int& prefix=32, quint32 mtu=1500);

    void addRoute (const QString& dev, const QString& addr, const int& prefix=32, const QString& gw="");
    void delRoute (const QString& dev, const QString& addr, const int& prefix, const QString& gw="");

    QHostAddress CalculateSubNet(const QString& addr, const int& pref);

private:
    Device *mDevice;
};

#endif // DEVICECOMMAND_H
