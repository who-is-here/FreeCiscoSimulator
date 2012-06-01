#ifndef ROUTERSHOWCOMMAND_H
#define ROUTERSHOWCOMMAND_H

#include "devicecommand.h"

class RouterShowCommand : public DeviceCommand
{
    Q_OBJECT
    public:
        explicit RouterShowCommand(Device* device);
        void exec(const QString& myArgsStr);
    private:
        QString cmdShow_ip_int(const QString& if_name = "") const;
    signals:
        void ShowAvailableCommands(const QStringList& lst, QString str);
        void MarkError(const QString& str);
        void KeepCommandRequest();
};

#endif // ROUTERSHOWCOMMAND_H
