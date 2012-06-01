#ifndef ROUTER_H
#define ROUTER_H

#include "Device.h"

class router : public Device
{
    Q_OBJECT
    public:
        explicit router (const QString& name = "", const unsigned int& count = 2);

        struct ACL_rule {
          QString action;
          QString proto;
          QHostAddress src;
          QHostAddress dst;
          QString type;
          quint16 port;
        };
        struct ACL {
            QString name;
            QString type;
            QList<ACL_rule> rules;
        };

        QHash<QString, ACL> acls() const
            { return ACLs; }
        void AddACL (const QString& name, const QString& type);
        void DelACL (const QString& name, const QString& type);

    private:
        void CreatePorts(const quint8& count);
        void InitializeInterfaces(const quint8& count);

        QString enable_password;

        QStringList prompt_state;

        QHash<QString, ACL> ACLs;

        void cmdEnable(void);
        void cmdExit(void);
        void cmdEnd(void);
        void cmdShow(QString myArgsStr);
        void cmdConfigure(QString myArgsStr);
        void cmdInterface(QString myArgsStr);
        void cmdIp(QString myArgsStr);
        void cmdIp2(QString myArgsStr);
        void cmdAddRule(QString myArgsStr);
        void cmdShutdown();
        void ShowHelp();

    public slots:
        void onExecuteCommand(const QString& cmd);
        void MarkError(const QString& cmd);
        void ShowAvailableCommands(const QStringList& cmds, QString cmd);
        void RequestToChangePrompt(const QString& newPrompt);
};

#endif // ROUTER_H
