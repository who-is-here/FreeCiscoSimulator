#ifndef GLOBAL_VARIABLES_H
#define GLOBAL_VARIABLES_H

#include <QString>
QString lastDeviceID = "0x0";
QString lastMACAddressOctet = "0x0";
#include "mainwindow.h"
MainWindow* mw = NULL;
//#include "Device.h"
//QHash<Device::DeviceType, QHash<QString, Device::cmdPlugin> > plugins;
quint16 start_port = 32978;

#endif // GLOBAL_VARIABLES_H
