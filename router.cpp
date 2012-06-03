#include "router.h"
#include "deviceport.h"
#include "routeripconfigurecommand.h"
#include "routeriproutecommand.h"
#include "routershowcommand.h"

router::router(const QString &name, const unsigned int &count) : Device (name)
{
    type = dev_router;
    CreatePorts(count);
    InitializeInterfaces(count);

    enable_password = "secret";

    prompt_state.clear();

    allow_forward = true;
}

void router::CreatePorts(const quint8 &count)
{
    devicePort *port;
    ports.clear();
    for(quint8 i=1;i<=count;i++)
    {
        port = new devicePort;
        port->number = i;
        port->name = QString("FastEthernet0/%1").arg(i-1);
        port->busy = false;
        ports.insert(QString("FastEthernet0/%1").arg(i-1), port);
    }
    foreach (QString key, ports.keys())
    {
        connect(this, SIGNAL(doSend(quint8,ether_frame&)), ports[key], SLOT(doSend(quint8,ether_frame&)));
        connect(ports[key], SIGNAL(doRecieve(ether_frame&,quint8)), this, SLOT(PacketRecieved(ether_frame&,quint8)));
    }
}

void router::InitializeInterfaces(const quint8& count)
{
    deviceInterface link;
    mInterfaces.clear();
    for(quint8 i=1;i<=count;i++)
    {
        foreach (QString key, ports.keys())
        {
            if (ports[key]->number == i)
            {
                link.AssignedPort = ports[key];
            }
        }
        link.name = QString("FastEthernet0/%1").arg(i-1);
        link.mtu = 1500;
        link.state = true;
        extern QString lastMACAddressOctet;
        bool ok;
        int buf = lastMACAddressOctet.toInt(&ok, 16);
        buf += 1;
        if (ok)
        {
            QString str = QString::number(buf, 16);
            if (str.length() == 1)
                link.hwaddr = QString("00:00:00:11:22:0").append(str);
            else if (str.length() == 1)
                link.hwaddr = QString("00:00:00:11:22:").append(str);
            lastMACAddressOctet = str;
        }
        link.address.setIp(QHostAddress(QString("192.168.0.%1").arg(i)));
        link.address.setPrefixLength(24);
        link.address.setBroadcast(CalculateBroadCast(link.address.ip().toString(),link.address.prefixLength()));
        mInterfaces.insert(link.name, link);
        addRoute(link.name, link.address.ip().toString(), link.address.prefixLength());
    }
}

void router::onExecuteCommand(const QString &cmd)
{
    CommandToBeInterrupted = "";

    QString c = cmd.simplified();
    QString command = c.section(" ",0,0);
    cur_cmd = command;
    QStringList myArgs = c.section(" ",1).split(" ");
    QString myArgsStr = c.section(" ", 1);

    //            Как-то еще надо реализовать READ с консоли

    isNegated = false;

    if (command == "?")
    {
        ShowHelp();
        return;
    }

    if (QString("exit").startsWith(command, Qt::CaseInsensitive) && command.size() > 1)
    {
        this->cmdExit();
        return;
    }
    QStringList available_cmd;
    if (prompt_state.isEmpty())
    {
        available_cmd << "exit" << "enable";
        QRegExp rx("^(exit|enable)", Qt::CaseInsensitive);
        rx.exactMatch(command);
        if (QString("enable").startsWith(command, Qt::CaseInsensitive) && command.size() > 1)
            this->cmdEnable();
        else if (command.endsWith("?"))
        {
            ShowAvailableCommands(available_cmd, command);
        }
        else if (myArgs.first() == "?")
        {
            // I don't detect situation named "% Ambiguous command"
            emit ConsoleWrite("% Unrecognized command");
            emit CommandReturn();
            return;
        }
        else if (rx.matchedLength() > 0)
        {
            MarkError(command.left(rx.matchedLength()));
        }
        else
        {
            emit ConsoleWrite("command unknown");
            emit CommandReturn();
        }
    }
    else if (prompt_state.last() == "# ")
    {
        available_cmd << "configure" << "show" << "exit";
        QRegExp rx("^(configure|show|exit)", Qt::CaseInsensitive);
        rx.exactMatch(command);
        if (QString("configure").startsWith(command, Qt::CaseInsensitive) && command.size() > 3)
        {
            this->cmdConfigure(myArgsStr);
        }
        else if (QString("show").startsWith(command, Qt::CaseInsensitive) && command.size() > 1)
        {
            this->cmdShow(myArgsStr);
        }
        else if (command.endsWith("?"))
        {
            ShowAvailableCommands(available_cmd, command);
        }
        else if (myArgs.first() == "?")
        {
            // I don't detect situation named "% Ambiguous command"
            emit ConsoleWrite("% Unrecognized command");
            emit CommandReturn();
            return;
        }
        else if (rx.matchedLength() > 0)
        {
            MarkError(command.left(rx.matchedLength()));
        }
        else
        {
            emit ConsoleWrite("command unknown");
            emit CommandReturn();
        }
    }
    else if (prompt_state.last() == "(config)# ")
    {
        available_cmd << "interface" << "end" << "exit" << "ip" << "no";
        QRegExp rx("^(interface|end|exit|ip|no)", Qt::CaseInsensitive);
        if (command == "no")
        {
            cur_cmd += myArgs.at(0);
            command = myArgs.at(0);
            myArgs.removeFirst();
            if (myArgs.isEmpty())
            {
                myArgs.append("");
            }
            myArgsStr.remove(QRegExp("^\\w*\\s*"));
            isNegated = true;
        }
        if (QString("interface").startsWith(command, Qt::CaseInsensitive) && command.size()>1)
        {
            this->cmdInterface(myArgsStr);
        }
        else if (QString("ip").startsWith(command, Qt::CaseInsensitive) && command.size()>1)
        {
            this->cmdIp2(myArgsStr);
        }
        else if (QString("end").startsWith(command, Qt::CaseInsensitive) && command.size()>1)
        {
            this->cmdEnd();
        }
        else if (command.endsWith("?"))
        {
            ShowAvailableCommands(available_cmd, command);
        }
        else if (myArgs.first() == "?")
        {
            // I don't detect situation named "% Ambiguous command"
            emit ConsoleWrite("% Unrecognized command");
            emit CommandReturn();
            return;
        }
        else
        {
            rx.exactMatch(command);
            MarkError(command.left(rx.matchedLength()));
        }
    }
    else if (prompt_state.last() == "(config-if)# ")
    {
        available_cmd << "end" << "exit" << "ip" << "no" << "shutdown";
        QRegExp rx("^(end|exit|ip|no|shutdown)", Qt::CaseInsensitive);
        rx.exactMatch(command);

        if (command == "no")
        {
            cur_cmd += myArgs.at(0);
            command = myArgs.at(0);
            myArgs.removeFirst();
            if (myArgs.isEmpty())
            {
                myArgs.append("");
            }
            myArgsStr.remove(QRegExp("^\\w*\\s*"));
            isNegated = true;
        }
        if (QString("ip").startsWith(command, Qt::CaseInsensitive) && command.size()>1)
        {
            this->cmdIp(myArgsStr);
        }
        else if (QString("shutdown").startsWith(command, Qt::CaseInsensitive) && command.size()>2)
        {
            this->cmdShutdown();
        }
        else if (QString("end").startsWith(command, Qt::CaseInsensitive) && command.size()>1)
        {
            this->cmdEnd();
        }
        else if (command.endsWith("?"))
        {
            ShowAvailableCommands(available_cmd, command);
        }
        else if (myArgs.first() == "?")
        {
            // I don't detect situation named "% Ambiguous command"
            emit ConsoleWrite("% Unrecognized command");
            emit CommandReturn();
            return;
        }
        else
        {
            if (isNegated)
            {
                command = "no " + command;
            }
            MarkError(command.left(rx.matchedLength()));
        }
    }
    else if (prompt_state.last() == "(config-ext-acl)# ")
    {
        available_cmd << "deny" << "end" << "exit" << "no" << "permit";
        QRegExp rx("^(deny|end|exit|no|permit)", Qt::CaseInsensitive);
        rx.exactMatch(command);

        if (command == "no")
        {
            cur_cmd += myArgs.at(0);
            command = myArgs.at(0);
            myArgs.removeFirst();
            if (myArgs.isEmpty())
            {
                myArgs.append("");
            }
            myArgsStr.remove(QRegExp("^\\w*\\s*"));
            isNegated = true;
        }
        if (QString("permit").startsWith(command, Qt::CaseInsensitive) ||
            QString("deny").startsWith(command, Qt::CaseInsensitive))
        {
            this->cmdAddRule(myArgsStr);
        }
        else
        {
            if (isNegated)
            {
                command = "no " + command;
            }
            MarkError(command.left(rx.matchedLength()));
        }
    }
    else
    {
        emit ConsoleWrite("unknown situation");
        emit CommandReturn();
    }
}

void router::cmdExit()
{
    if (!prompt_state.isEmpty())
    {
        if (prompt_state.size() > 1)
        {
            prompt_state.removeLast();
            emit ChangePrompt(prompt_state.last());
            emit CommandReturn();
        }
        else
        {
            prompt_state.clear();
            emit ChangePrompt("> ");
            emit CommandReturn();
        }
    }
    else
    {
        emit DisconnectConsole();
    }

}

void router::cmdEnd()
{
    prompt_state.clear();
    prompt_state.append("# ");
    emit ChangePrompt(prompt_state.last());
    emit CommandReturn();
}

void router::cmdEnable()
{
    if (!prompt_state.isEmpty())
    {
        emit CommandReturn();
    }
    else
    {
        emit ConsoleWrite("Password:");
        // *****************
        // HOW TO REALIZE READING DATA FROM CONSOLE???
        // *****************
        prompt_state.clear();
        prompt_state.append("# ");
        emit ChangePrompt(prompt_state.last());
        emit CommandReturn();
    }
}

void router::cmdShow(QString myArgsStr)
{
    Command* cmd = new RouterShowCommand(this);
    connect(cmd, SIGNAL(Write(QString)), this, SIGNAL(ConsoleWrite(QString)));
    connect(cmd, SIGNAL(Finished()), this, SIGNAL(CommandReturn()));
    connect(cmd, SIGNAL(ShowAvailableCommands(QStringList,QString)), this, SLOT(ShowAvailableCommands(QStringList,QString)));
    connect(cmd, SIGNAL(MarkError(QString)), this, SLOT(MarkError(QString)));
    connect(cmd, SIGNAL(KeepCommandRequest()), this, SIGNAL(KeepCommandRequest()));
    cmd->exec(myArgsStr);
    delete cmd;
}

void router::cmdConfigure(QString myArgsStr)
{
    QStringList myArgs = myArgsStr.split(" ");
    QStringList available_cmd;
    if (myArgsStr.endsWith("?"))
    {
        switch (myArgs.size())
        {
            case 1:
                available_cmd << "terminal";
                ShowAvailableCommands(available_cmd, myArgs.at(0));
                break;
            default:
                emit ConsoleWrite("% Unrecognized command");
                emit CommandReturn();
        }
        return;
    }
    if (QString(myArgs.at(0)).isEmpty())
    {
        emit ConsoleWrite("Configuring from terminal, memory, or network [terminal]? terminal");
        prompt_state.append("(config)# ");
        emit ChangePrompt(prompt_state.last());
        emit CommandReturn();
    }
    else if (QString("terminal").startsWith(myArgs.at(0)))
    {
        emit ConsoleWrite("Enter configuration commands, one per line.  End with CNTL/Z.");
        prompt_state.append("(config)# ");
        emit ChangePrompt(prompt_state.last());
        emit CommandReturn();
    }
    else
    {
        MarkError("configure ");
    }
}

void router::cmdInterface(QString myArgsStr)
{
    QStringList myArgs = myArgsStr.split(" ");
    QStringList available_cmd;
    if (myArgsStr.endsWith("?"))
    {
        switch (myArgs.size())
        {
            case 1:
                available_cmd << "FastEthernet";
                ShowAvailableCommands(available_cmd, myArgs.at(0));
                break;
            case 2:
                if (myArgs.at(1) == "?")
                {
                    emit ConsoleWrite("  <0-9>  FastEthernet interface number");
                }
                else if (QRegExp("\\d/\\d\\?").exactMatch(myArgs.at(1)))
                {
                    emit ConsoleWrite("/    -");
                }
                else
                {
                    emit ConsoleWrite("/");
                }
                emit KeepCommandRequest();
                emit CommandReturn();
                break;
            default:
                emit ConsoleWrite("% Unrecognized command");
                emit CommandReturn();
        }
        return;
    }
    QString str = "";
    QRegExp rx("^F(a(s(t(E(t(h(e(r(n(e(t)?)?)?)?)?)?)?)?)?)?)? *\\d/\\d$", Qt::CaseInsensitive);
    if (myArgsStr.isEmpty())
    {
        str = "% Incomplete command.";
        emit ConsoleWrite(str);
        emit CommandReturn();
    }
    else if (rx.exactMatch(myArgsStr))
    {
        bool ifExist = false;
        foreach (deviceInterface iface, mInterfaces)
        {
            if (myArgsStr.right(3) == iface.name.right(3))
            {
                ifExist = true;
                config_if = iface.name;
                prompt_state.append("(config-if)# ");
                emit ChangePrompt(prompt_state.last());
                emit CommandReturn();
            }
        }
        if (!ifExist)
        {
            str = "%Invalid interface type and number";
            emit ConsoleWrite(str);
            emit CommandReturn();
        }
    }
    else
    {
        MarkError(cur_cmd + " " + myArgsStr.left(rx.matchedLength()));
    }
}

void router::cmdIp(QString myArgsStr)
{
    Command* cmd = new RouterIpConfigureCommand(this);
    connect(cmd, SIGNAL(Write(QString)), this, SIGNAL(ConsoleWrite(QString)));
    connect(cmd, SIGNAL(Finished()), this, SIGNAL(CommandReturn()));
    connect(cmd, SIGNAL(ShowAvailableCommands(QStringList,QString)), this, SLOT(ShowAvailableCommands(QStringList,QString)));
    connect(cmd, SIGNAL(MarkError(QString)), this, SLOT(MarkError(QString)));
    connect(cmd, SIGNAL(KeepCommandRequest()), this, SIGNAL(KeepCommandRequest()));
    cmd->exec(myArgsStr);
    delete cmd;
}

void router::cmdIp2(QString myArgsStr)
{
    Command* cmd = new RouterIpRouteCommand(this);
    connect(cmd, SIGNAL(Write(QString)), this, SIGNAL(ConsoleWrite(QString)));
    connect(cmd, SIGNAL(Finished()), this, SIGNAL(CommandReturn()));
    connect(cmd, SIGNAL(ShowAvailableCommands(QStringList,QString)), this, SLOT(ShowAvailableCommands(QStringList,QString)));
    connect(cmd, SIGNAL(MarkError(QString)), this, SLOT(MarkError(QString)));
    connect(cmd, SIGNAL(KeepCommandRequest()), this, SIGNAL(KeepCommandRequest()));
    connect(cmd, SIGNAL(ChangePrompt(QString)), this, SLOT(RequestToChangePrompt(QString)));
    cmd->exec(myArgsStr);
    delete cmd;
}

void router::RequestToChangePrompt(const QString &newPrompt)
{
    prompt_state.append(newPrompt);
    emit ChangePrompt(newPrompt);
}

void router::cmdShutdown()
{
    if (isNegated)
    {
        if (!mInterfaces[config_if].state)
        {
            QString str = QString("%LINK-5-CHANGED: Interface ").append(config_if).append(", changed state to up\n\n%LINEPROTO-5-UPDOWN: Line protocol on Interface ").append(
                config_if).append(", changed state to up");
            emit ConsoleWrite(str);
        }
        SetInterfaceState(config_if, true);
    }
    else
    {
        if (mInterfaces[config_if].state)
        {
            QString str = QString("%LINK-5-CHANGED: Interface ").append(config_if).append(", changed state to administratively down\n\n%LINEPROTO-5-UPDOWN: Line protocol on Interface ").append(
                config_if).append(", changed state to down");
            emit ConsoleWrite(str);
        }
        SetInterfaceState(config_if, false);
    }

    emit CommandReturn();
}

void router::MarkError(const QString &cmd)
{
    if (prompt_state.isEmpty())
    {
        emit ConsoleWriteHTML("<font color=\"black\">> " + cmd + "</font>^");
    }
    else
    {
        emit ConsoleWriteHTML("<font color=\"black\">" + prompt_state.last() + cmd + "</font>^");
    }
    emit ConsoleWrite("% Invalid input detected at '^' marker.\n");
    emit CommandReturn();
}

void router::ShowAvailableCommands(const QStringList &cmds, QString cmd)
{
    QString str = "";
    foreach (QString s, cmds)
    {
        cmd.remove("?").simplified();
        if (s.startsWith(cmd, Qt::CaseInsensitive))
        {
            if (str.isEmpty())
            {
                str = s;
            }
            else
            {
                str.append(" ").append(s);
            }
        }
    }
    if (str.isEmpty())
    {
        str = "% Unrecognized command";
    }
    emit ConsoleWrite(str);
    emit KeepCommandRequest();
    emit CommandReturn();
}

void router::ShowHelp()
{
    QString str = "";
    if (prompt_state.isEmpty())
    {
        str = "  enable      Turn on privileged commands\n"
              "  exit        Exit from the EXEC";
    }
    else if (prompt_state.last() == "# ")
    {
        str = "  configure   Enter configuration mode\n"
              "  exit        Exit from the EXEC\n"
              "  show        Show running system information";
    }
    else if (prompt_state.last() == "(config)# ")
    {
        str = "  end                Exit from configure mode\n"
              "  exit               Exit from configure mode\n"
              "  interface          Select an interface to configure\n"
              "  ip                 Global IP configuration subcommands\n"
              "  no                 Negate a command or set its defaults";
    }
    else if (prompt_state.last() == "(config-if)# ")
    {
        str = "  end                Exit from configure mode\n"
              "  exit               Exit from interface configuration mode\n"
              "  ip                 Interface Internet Protocol config commands\n"
              "  no                 Negate a command or set its defaults\n"
              "  shutdown           Shutdown the selected interface";
    }
    else if (prompt_state.last() == "(config-ext-acl)# ")
    {
        str = "  deny     Specify packets to reject\n"
              "  exit     Exit from access-list configuration mode\n"
              "  no       Negate a command or set its defaults\n"
              "  permit   Specify packets to forward";
    }
    emit ConsoleWrite(str);
    emit CommandReturn();
}

void router::AddACL(const QString &name, const QString &type)
{
    foreach (ACL a, ACLs)
    {
        if (name == a.name && type == a.type)
        {
            return;
        }
    }

    ACL a;
    a.name = name;
    a.type = type;
    a.rules.clear();
    ACLs.insert(name, a);
}

void router::DelACL(const QString &name, const QString &type)
{
    foreach (QString key, ACLs.keys())
    {
        if (name == key && type == ACLs[key].type)
        {
            ACLs[key].rules.clear();
            ACLs.remove(key);
        }
    }
}

void router::cmdAddRule(QString myArgsStr)
{
    QStringList myArgs = myArgsStr.split(" ");
    QStringList available_cmd;
    if (myArgsStr.endsWith("?"))
    {
        emit KeepCommandRequest();
        switch (myArgs.size())
        {
        case 1:
            if (myArgs.at(0) == "?")
            {
                emit ConsoleWrite("  icmp   Internet Control Message Protocol\n"
                                  "  ip     Any Internet Protocol\n"
                                  "  tcp    Transmission Control Protocol\n"
                                  "  udp    User Datagram Protocol");
                emit CommandReturn();
            }
            else
            {
                available_cmd << "icmp" << "ip" << "tcp" << "udp";
                ShowAvailableCommands(available_cmd, myArgs.at(0));
            }
            break;
        case 2:
//            if (QString("icmp").startsWith(myArgs.at(0)) && QString(myArgs.at(0)).size() > 1)
            if (QRegExp("(ic(mp?)?|ip|t(cp?)?|u(dp?)?) .*").exactMatch(myArgsStr))
            {
                if (myArgs.at(1) == "?")
                {
                    emit ConsoleWrite("  A.B.C.D  Source address\n"
                                      "  any      Any source host");
                }
                else if (QRegExp("\\d(\\d(\\d)?)?(\\.(\\d(\\d(\\d)?)?)?)?(\\.(\\d(\\d(\\d)?)?)?)?(\\.(\\d(\\d(\\d)?)?)?)?\\?").exactMatch(myArgs.at(1)))
                {
                    QStringList octets = QString(myArgs.at(1)).split(".");
                    bool isAddress = true;
                    foreach (QString s, octets)
                    {
                        if (s.remove("?").isEmpty())
                        {
                            continue;
                        }
                        bool ok;
                        int x = s.toInt(&ok);
                        if (!ok || x>255)
                        {
                            isAddress = false;
                        }
                    }
                    if (isAddress)
                    {
                        emit ConsoleWrite("A.B.C.D");
                    }
                    else
                    {
                        emit ConsoleWrite("% Unrecognized command");
                    }
                }
                else if (QRegExp("a(ny?)?\\?", Qt::CaseInsensitive).exactMatch(myArgs.at(1)))
                {
                    emit ConsoleWrite("any");
                }
                else
                {
                    emit ConsoleWrite("% Unrecognized command");
                }
            }
            emit CommandReturn();
            break;
        case 3:
            if (QRegExp("(ic(mp?)?|ip|t(cp?)?|u(dp?)?) +(\\d(\\d(\\d)?)?\\.\\d(\\d(\\d)?)?\\.\\d(\\d(\\d)?)?\\.\\d(\\d(\\d)?)?|any) +.*",
                        Qt::CaseInsensitive).exactMatch(myArgsStr))
            {
                if (myArgs.at(2) == "?")
                {
                    emit ConsoleWrite("  A.B.C.D  Source address\n"
                                      "  any      Any source host");
                }
                else if (QRegExp("\\d(\\d(\\d)?)?(\\.(\\d(\\d(\\d)?)?)?)?(\\.(\\d(\\d(\\d)?)?)?)?(\\.(\\d(\\d(\\d)?)?)?)?\\?").exactMatch(myArgs.at(2)))
                {
                    QStringList octets = QString(myArgs.at(2)).split(".");
                    bool isAddress = true;
                    foreach (QString s, octets)
                    {
                        if (s.remove("?").isEmpty())
                        {
                            continue;
                        }
                        bool ok;
                        int x = s.toInt(&ok);
                        if (!ok || x>255)
                        {
                            isAddress = false;
                        }
                    }
                    if (isAddress)
                    {
                        emit ConsoleWrite("A.B.C.D");
                    }
                    else
                    {
                        emit ConsoleWrite("% Unrecognized command");
                    }
                }
                else if (QRegExp("a(ny?)?\\?", Qt::CaseInsensitive).exactMatch(myArgs.at(2)))
                {
                    emit ConsoleWrite("any");
                }
                else
                {
                    emit ConsoleWrite("% Unrecognized command");
                }
            }
            else
            {
                emit ConsoleWrite("% Unrecognized command");
            }
            emit CommandReturn();
            break;
        case 4:
            break;
        case 5:
            break;
        default:
            break;
        }
    }
}
