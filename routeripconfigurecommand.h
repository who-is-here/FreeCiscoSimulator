#ifndef ROUTERIPCONFIGURECOMMAND_H
#define ROUTERIPCONFIGURECOMMAND_H

#include "devicecommand.h"

class RouterIpConfigureCommand : public DeviceCommand
{
    Q_OBJECT
    public:
        explicit RouterIpConfigureCommand(Device* device);
        void exec(const QString& myArgsStr);
    signals:
        void ShowAvailableCommands(const QStringList& lst, QString str);
        void MarkError(const QString& str);
        void KeepCommandRequest();
};

#endif // ROUTERIPCONFIGURECOMMAND_H
