#ifndef ROUTERIPROUTECOMMAND_H
#define ROUTERIPROUTECOMMAND_H

#include "devicecommand.h"

class RouterIpRouteCommand : public DeviceCommand
{
    Q_OBJECT
    public:
        explicit RouterIpRouteCommand(Device* device);
        void exec(const QString& myArgsStr);
    signals:
        void ShowAvailableCommands(const QStringList& lst, QString str);
        void MarkError(const QString& str);
        void KeepCommandRequest();
        void ChangePrompt(const QString& newPrompt);
};

#endif // ROUTERIPROUTECOMMAND_H
