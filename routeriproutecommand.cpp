#include "routeriproutecommand.h"
#include "Device.h"
#include "router.h"

RouterIpRouteCommand::RouterIpRouteCommand(Device* device) :
        DeviceCommand("ip", device)
{
}

void RouterIpRouteCommand::exec(const QString &myArgsStr)
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
                emit Write("  access-list       Named access-list\n"
                           "  route               Establish static routes");
                emit Finished();
            }
            else
            {
                available_cmd << "route" << "access-list";
                emit ShowAvailableCommands(available_cmd, myArgs.at(0));
            }
            break;
        case 2:
            if (QString("route").startsWith(myArgs.at(0), Qt::CaseInsensitive) && QString(myArgs.at(0)).size() > 1)
            {

                if (myArgs.at(1) == "?")
                {
                    emit Write("  A.B.C.D  Destination prefix");
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
            else if (QString("access-list").startsWith(myArgs.at(0), Qt::CaseInsensitive) && QString(myArgs.at(0)).size() > 1)
            {
                if (myArgs.at(1) == "?")
                {
                    emit Write("  extended  Extended Access List");
                    emit Finished();
                }
                else
                {
                    available_cmd << "extended";
                    emit ShowAvailableCommands(available_cmd, myArgs.at(1));
                }
            }
            else
            {
                emit Write("% Unrecognized command");
                emit Finished();
                break;
            }
            break;
        case 3:
            if (QRegExp("ro(u(t(er?)?)?)? \\d(\\d(\\d)?)?\\.\\d(\\d(\\d)?)?\\.\\d(\\d(\\d)?)?\\.\\d(\\d(\\d)?)? .*", Qt::CaseInsensitive).exactMatch(myArgsStr))
            {

                if (myArgs.at(2) == "?")
                {
                    emit Write("  A.B.C.D  Destination prefix mask");
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
            }
            else if (QString("access-list").startsWith(myArgs.at(0), Qt::CaseInsensitive) && QString(myArgs.at(0)).size() > 1)
            {
                if (QString("extended").startsWith(myArgs.at(1), Qt::CaseInsensitive))
                {
                    if (myArgs.at(2) == "?")
                    {
                        emit Write("  <100-199>  Extended IP access-list number\n"
                                   "  WORD          name");
                    }
                    else
                    {
                        if (QRegExp("\\d{3}\\?").exactMatch(myArgs.at(2)))
                        {
                            if (QString(myArgs.at(2)).remove("?").toInt() > 99 &&
                                QString(myArgs.at(2)).remove("?").toInt() < 200)
                            {
                                emit Write("<100-199>  WORD");
                            }
                            else
                            {
                                emit Write("WORD");
                            }
                        }
                        else
                        {
                            emit Write("WORD");
                        }
                    }
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
            if (QRegExp("ro(u(t(er?)?)?)? \\d(\\d(\\d)?)?\\.\\d(\\d(\\d)?)?\\.\\d(\\d(\\d)?)?\\.\\d(\\d(\\d)?)? "
                        "\\d(\\d(\\d)?)?\\.\\d(\\d(\\d)?)?\\.\\d(\\d(\\d)?)?\\.\\d(\\d(\\d)?)? .*",
                        Qt::CaseInsensitive).exactMatch(myArgsStr))
            {
                if (myArgs.at(3) == "?")
                {
                    emit Write("  A.B.C.D          Forwarding router's address");
                }
                else if (QRegExp("\\d(\\d(\\d)?)?(\\.(\\d(\\d(\\d)?)?)?)?(\\.(\\d(\\d(\\d)?)?)?)?(\\.(\\d(\\d(\\d)?)?)?)?\\?").exactMatch(myArgs.at(3)))
                {
                    QStringList octets = QString(myArgs.at(3)).split(".");
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
            }
            else if (QRegExp("ac(c(e(s(s(-(l(i(st?)?)?)?)?)?)?)?)? +e(x(t(e(n(d(ed?)?)?)?)?)?)? +\\w+ +\\?", Qt::CaseInsensitive).exactMatch(myArgsStr))
            {
                emit Write("  <cr>");
            }
            else
            {
                emit Write("% Unrecognized command");
            }
            emit Finished();
            break;
        case 5:
            if (QRegExp("^ro(u(t(er?)?)?)? +\\d(\\d(\\d)?)?\\.\\d(\\d(\\d)?)?\\.\\d(\\d(\\d)?)?\\.\\d(\\d(\\d)?)? +"
                        "\\d(\\d(\\d)?)?\\.\\d(\\d(\\d)?)?\\.\\d(\\d(\\d)?)?\\.\\d(\\d(\\d)?)? +"
                        "\\d(\\d(\\d)?)?\\.\\d(\\d(\\d)?)?\\.\\d(\\d(\\d)?)?\\.\\d(\\d(\\d)?)? +\\?",
                        Qt::CaseInsensitive).exactMatch(myArgsStr))
            {
                emit Write("  <cr>");
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
    QRegExp rx_0("^(access-list|route)", Qt::CaseInsensitive);
    if (QString("route").startsWith(myArgs.at(0), Qt::CaseInsensitive) && QString(myArgs.at(0)).size() > 1)
    {
        QRegExp rx("^ro(u(t(er?)?)?)? \\d(\\d(\\d)?)?\\.\\d(\\d(\\d)?)?\\.\\d(\\d(\\d)?)?\\.\\d(\\d(\\d)?)? \\d(\\d(\\d)?)?\\.\\d(\\d(\\d)?)?\\.\\d(\\d(\\d)?)?\\.\\d(\\d(\\d)?)? \\d(\\d(\\d)?)?\\.\\d(\\d(\\d)?)?\\.\\d(\\d(\\d)?)?\\.\\d(\\d(\\d)?)?", Qt::CaseInsensitive);
        if (rx.exactMatch(myArgsStr))
        {
            if (QHostAddress(myArgs.at(1)) == QHostAddress::Null)
            {
                emit MarkError(device()->cur_cmd + " " + myArgsStr.section(" ", 0, 0) + " ");
            }
            else if (QHostAddress(myArgs.at(2)) == QHostAddress::Null)
            {
                emit MarkError(device()->cur_cmd + " " + myArgsStr.section(" ", 0, 1) + " ");
            }
            else if (QHostAddress(myArgs.at(3)) == QHostAddress::Null)
            {
                emit MarkError(device()->cur_cmd + " " + myArgsStr.section(" ", 0, 2) + " ");
            }
            else
            {
                if (QRegExp("1{,32}0{,32}").exactMatch(QString::number(QHostAddress(myArgs.at(2)).toIPv4Address(), 2)))
                {
                    if (QHostAddress(myArgs.at(1)) == CalculateSubNet(myArgs.at(1), QString::number(
                            QHostAddress(myArgs.at(2)).toIPv4Address(), 2).count("1")))
                    {
                        QString def_dev("");
                        foreach (Device::route r, device()->routes())
                        {
                            if (r.net == QHostAddress::Any && r.prefix == 0)
                            {
                                def_dev = r.dev;
                            }
                            else if (QHostAddress(myArgs.at(3)).isInSubnet(r.net, r.prefix))
                            {
                                // Choose the first accepted device, without see on metric
                                if (device()->isNegated)
                                {
                                    device()->delRoute(r.dev, myArgs.at(1), QString::number(
                                            QHostAddress(myArgs.at(2)).toIPv4Address(), 2).count("1"), myArgs.at(3));
                                }
                                else
                                {
                                    device()->addRoute(r.dev, myArgs.at(1), QString::number(
                                            QHostAddress(myArgs.at(2)).toIPv4Address(), 2).count("1"), myArgs.at(3));
                                }
                                emit Finished();
                                return;
                            }
                        }
                        if (!def_dev.isNull())
                        {
                            if (device()->isNegated)
                            {
                                device()->delRoute(def_dev, myArgs.at(1), QString::number(
                                        QHostAddress(myArgs.at(2)).toIPv4Address(), 2).count("1"), myArgs.at(3));
                            }
                            else
                            {
                                device()->addRoute(def_dev, myArgs.at(1), QString::number(
                                        QHostAddress(myArgs.at(2)).toIPv4Address(), 2).count("1"), myArgs.at(3));
                            }
                        }
                    }
                    else
                    {
                        emit Write("%Inconsistent address and mask");
                    }
                }
                else
                {
                    emit Write("%Inconsistent address and mask");
                }
                emit Finished();
            }
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
            else if (myArgsStr.left(rx.matchedLength()).count(" ") == 3)
            {
                emit MarkError(device()->cur_cmd + " " + myArgsStr.section(" ", 0, 2) + " ");
            }
            else
            {
                emit MarkError(device()->cur_cmd + " " + myArgsStr.left(rx.matchedLength()));
            }
        }
    }
    else if (QString("access-list").startsWith(myArgs.at(0), Qt::CaseInsensitive) && QString(myArgs.at(0)).size() > 1)
    {
        QRegExp rx("ac(c(e(s(s(-(l(i(st?)?)?)?)?)?)?)?)? +e(x(t(e(n(d(ed?)?)?)?)?)?)? +\\w+", Qt::CaseInsensitive);
        if (rx.exactMatch(myArgsStr))
        {
            router* r = static_cast<router*>(device());
            if (device()->isNegated)
            {
                r->DelACL(myArgs.at(2), "extended");
            }
            else
            {
                r->AddACL(myArgs.at(2), "extended");
                emit ChangePrompt("(config-ext-acl)# ");
            }
            emit Finished();
        }
        else
        {
            emit MarkError(device()->cur_cmd + " " + myArgsStr.left(rx.matchedLength()));
        }
    }
    else
    {
        rx_0.exactMatch(myArgs.at(0));
        emit MarkError(device()->cur_cmd + " " + QString(myArgs.at(0)).left(rx_0.matchedLength()));
    }
}
