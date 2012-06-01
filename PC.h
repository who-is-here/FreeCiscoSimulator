#ifndef PC_H
#define PC_H

#define LINUX "GNU/Linux"
#define WINDOWS "MS Windows"
#define MAC = "Mac OS X"
#define FBSD "FreeBSD"
#define OBSD = "OpenBSD"
#define UNIX = "Unix"


#include "Device.h"

class PC : public Device
{
    Q_OBJECT
    public:
        //PC () : Device () {SetOSType();}
        //PC (const QString& name) : Device (name) {SetOSType();}
        explicit PC (const QString& name = "", const unsigned int& count = 1);

        QString GetOSType ();
        QString SetOSType (const QString& os = "GNU/Linux");
        bool IsLinux ();
        bool IsWindows ();

    private:
        void CreatePorts(const quint8& count);
        void InitializeInterfaces(const quint8& count);

        QString OSType;
        void ip (QString& args);
        void ping (QStringList& dst);
    public slots:
        void onExecuteCommand(const QString& cmd);
};

#endif // PC_H
