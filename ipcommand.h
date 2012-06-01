#ifndef IPCOMMAND_H
#define IPCOMMAND_H

#include "devicecommand.h"

class PCIpCommand : public DeviceCommand
{
public:
    explicit PCIpCommand(Device* device);
    void exec(const QString& args);
};

#endif // IPCOMMAND_H
