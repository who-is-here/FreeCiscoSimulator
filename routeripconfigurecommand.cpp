#include "routeripconfigurecommand.h"
#include "Device.h"

RouterIpConfigureCommand::RouterIpConfigureCommand(Device* device) :
        DeviceCommand("ip", device)
{
}

void RouterIpConfigureCommand::exec(const QString &myArgsStr)
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
                emit Write("  access-group        Specify access control for packets\n"
                           "  address               Set the IP address of an interface");
                emit Finished();
            }
            else
            {
                available_cmd << "address" << "access-group";
                emit ShowAvailableCommands(available_cmd, myArgs.at(0));
            }
            break;
        case 2:
            if (QString("address").startsWith(myArgs.at(0), Qt::CaseInsensitive) && QString(myArgs.at(0)).size() > 1)
            {

                if (myArgs.at(1) == "?")
                {
                    emit Write("  A.B.C.D  IP address");
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
                        emit Write("A.B.C.D");
                    }
                    else
                    {
                        emit Write("% Unrecognized command");
                    }
                }
                else
                {
                    emit Write("% Unrecognized command");
                }
                emit Finished();
            }
            else if (QString("access-group").startsWith(myArgs.at(0), Qt::CaseInsensitive) && QString(myArgs.at(0)).size() > 1)
            {
                if (myArgs.at(1) == "?")
                {
                    emit Write("  <1-199>  IP access list (standard or extended)\n"
                               "  WORD     Access-list name");
                }
                else
                {
                    emit Write("% Unrecognized command");
                }
                emit Finished();
            }
            else
            {
                emit Write("% Unrecognized command");
                emit Finished();
            }
            break;
        case 3:
            if (!QRegExp("ad(d(r(e(s(s)?)?)?)?)? \\d(\\d(\\d)?)?\\.\\d(\\d(\\d)?)?\\.\\d(\\d(\\d)?)?\\.\\d(\\d(\\d)?)? .*", Qt::CaseInsensitive).exactMatch(myArgsStr))
            {
                emit Write("% Unrecognized command");
                emit Finished();
                break;
            }
            if (myArgs.at(2) == "?")
            {
                emit Write("  A.B.C.D  IP subnet mask");
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
                    emit Write("A.B.C.D");
                }
                else
                {
                    emit Write("% Unrecognized command");
                }
            }
            else
            {
                emit Write("% Unrecognized command");
            }
            emit Finished();
            break;
            case 4:
            if (QRegExp("^ad(d(r(e(s(s)?)?)?)?)? \\d(\\d(\\d)?)?\\.\\d(\\d(\\d)?)?\\.\\d(\\d(\\d)?)?\\.\\d(\\d(\\d)?)? \\d(\\d(\\d)?)?\\.\\d(\\d(\\d)?)?\\.\\d(\\d(\\d)?)?\\.\\d(\\d(\\d)?)? +\\?", Qt::CaseInsensitive).exactMatch(myArgsStr))
            {
                emit Write("<CR>");
                emit Finished();
            }
            else
            {
                emit Write("% Unrecognized command");
                emit Finished();
            }
            break;
            default:
            emit Write("% Unrecognized command");
            emit Finished();
        }
        return;
    }
    if (device()->isNegated)
    {
        // TODO: Realize delete simple address "no ip address 192.168.0.2 255.255.255.0"

        QRegExp rx("ad(d(r(e(s(s)?)?)?)?)?");
        if (rx.exactMatch(myArgsStr))
        {
            ConfigureInterface(device()->config_if, "");
            emit Finished();
        }
        else
        {
            emit MarkError(device()->cur_cmd + " " + myArgsStr.left(rx.matchedLength()));
        }
    }
    else
    {
        QRegExp rx("ad(d(r(e(s(s)?)?)?)?)? \\d(\\d(\\d)?)?\\.\\d(\\d(\\d)?)?\\.\\d(\\d(\\d)?)?\\.\\d(\\d(\\d)?)? \\d(\\d(\\d)?)?\\.\\d(\\d(\\d)?)?\\.\\d(\\d(\\d)?)?\\.\\d(\\d(\\d)?)?", Qt::CaseInsensitive);
        if (rx.exactMatch(myArgsStr))
        {
            if (QRegExp("1{,32}0{,32}").exactMatch(QString::number(QHostAddress(myArgs.at(2)).toIPv4Address(), 2)))
            {
                if (QHostAddress(myArgs.at(1)) != QHostAddress::Null)
                {
                    ConfigureInterface(device()->config_if, myArgs.at(1), QString::number(QHostAddress(myArgs.at(2)).toIPv4Address(), 2).count("1"));
                }
                else
                {
                    emit MarkError(device()->cur_cmd + " " + myArgsStr.section(" ", 0, 0) + " ");
                    return;
                }
            }
            else
            {
                emit Write("Bad mask 0x" + QString::number(QHostAddress(myArgs.at(2)).toIPv4Address(), 16) + " for address " + myArgs.at(1));
            }
            emit Finished();
        }
        else
        {
            if (myArgsStr.left(rx.matchedLength()).count(" ") == 1)
            {
                emit MarkError(device()->cur_cmd + " " + myArgsStr.section(" ", 0, 0) + " ");
            }
            else if (myArgsStr.left(rx.matchedLength()).count(" ") == 2)
            {
                emit MarkError(device()->cur_cmd + " " + myArgsStr.section(" ", 0, 1) + " ");
            }
            else
            {
                emit MarkError(device()->cur_cmd + " " + myArgsStr.left(rx.matchedLength()));
            }
        }
    }
}
