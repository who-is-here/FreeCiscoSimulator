#ifndef COMMAND_H
#define COMMAND_H

#include <QObject>

class Device;
struct packet;

class Command: public QObject
{
    Q_OBJECT
public:
    QString commandName() const {return mCommandName;}

//    virtual void exec(const QStringList& args) = 0;
    virtual void exec(const QString& args) = 0;
    void interrupt()
        {}

signals:
    void Write(const QString& text);
    void Finished();
protected:
    Command(const QString& command):
        mCommandName(command)
    {}

protected:
    QString mCommandName;
};

class NetCommand: public Command
{
    Q_OBJECT
protected:
    NetCommand(const QString &command):
        Command(command)
    {}
public:
    bool isPacketForMe(packet& packet) {
        Q_UNUSED (packet)

        return false;
    }

    void ProcessPacket(packet& packet)
        {Q_UNUSED(packet)}
signals:
    void CreatedPacket(packet& packet);
};

#endif // COMMAND_H
