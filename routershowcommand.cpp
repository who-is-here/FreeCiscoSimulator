#include "routershowcommand.h"
#include "Device.h"
#include "deviceport.h"
#include "router.h"

RouterShowCommand::RouterShowCommand(Device* device) :
        DeviceCommand("show", device)
{
}

void RouterShowCommand::exec(const QString &myArgsStr)
{
    QString str = "";
    QStringList available_cmd;
    QStringList myArgs = myArgsStr.split(" ");
    if (QString(myArgs.at(0)).isEmpty())
    {
        myArgs.replace(0, "error");
    }
    QRegExp rx_0("^(interfaces|ip)", Qt::CaseInsensitive);
    if (myArgsStr.endsWith("?"))
    {
        emit KeepCommandRequest();
        switch (myArgs.size())
        {
        case 1:
            if (myArgs.at(0) == "?")
            {
                emit Write("  interfaces         Interface status and configuration\n"
                           "  ip                         IP information");
                emit Finished();
            }
            else
            {
                available_cmd << "interfaces" << "ip";
                ShowAvailableCommands(available_cmd, myArgs.at(0));
            }
            break;
        case 2:
            if (QString("interfaces").startsWith(myArgs.at(0), Qt::CaseInsensitive) && QString(myArgs.at(0)).size() > 1)
            {
                if (myArgs.at(1) == "?")
                {
                    emit Write("  FastEthernet      FastEthernet IEEE 802.3\n"
                               "  <cr>");
                    emit Finished();
                }
                else
                {
                    available_cmd << "FastEthernet";
                    ShowAvailableCommands(available_cmd, myArgs.at(1));
                }
            }
            else if (QString("ip").startsWith(myArgs.at(0), Qt::CaseInsensitive) && QString(myArgs.at(0)).size() > 1)
            {
                if (myArgs.at(1) == "?")
                {
                    emit Write("  access-lists  List access lists\n"
                               "  interface     IP interface status and configuration\n"
                               "  route              IP routing table");
                    emit Finished();
                }
                else
                {
                    available_cmd << "route" << "interface" << "access-list";
                    ShowAvailableCommands(available_cmd, myArgs.at(1));
                }
            }
            else
            {
                emit Write("% Unrecognized command");
                emit Finished();
            }
            break;
        case 3:
            if (QString("interfaces").startsWith(myArgs.at(0), Qt::CaseInsensitive) && QString(myArgs.at(0)).size() > 1)
            {
                if (QString("FastEthernet").startsWith(myArgs.at(1), Qt::CaseInsensitive) && QString(myArgs.at(1)).size() > 1)
                {
                    if (myArgs.at(2) == "?")
                    {
                        emit Write("  <0-9>  FastEthernet interface number");
                    }
                    else if (QRegExp("\\d/\\d\\?").exactMatch(myArgs.at(2)))
                    {
                        emit Write("/    -");
                    }
                    else
                    {
                        emit Write("/");
                    }
                    emit Finished();
                }
                else
                {
                    emit Write("% Unrecognized command");
                    emit Finished();
                }
            }
            else if (QString("ip").startsWith(myArgs.at(0), Qt::CaseInsensitive) && QString(myArgs.at(0)).size() > 1)
            {
                if (QString("route").startsWith(myArgs.at(1), Qt::CaseInsensitive) && QString(myArgs.at(1)).size() > 1)
                {
                    if (myArgs.at(2) == "?")
                    {
                        emit Write("  <cr>");
                    }
                    else
                    {
                        emit Write("% Unrecognized command");
                    }
                    emit Finished();
                }
                else if (QString("interface").startsWith(myArgs.at(1), Qt::CaseInsensitive) &&
                         QString(myArgs.at(1)).size() > 2)
                {
                    if (myArgs.at(2) == "?")
                    {
                        emit Write("  FastEthernet      FastEthernet IEEE 802.3\n"
                                   "  <cr>");
                        emit Finished();
                    }
                    else
                    {
                        available_cmd << "FastEthernet";
                        ShowAvailableCommands(available_cmd, myArgs.at(2));
                    }
                }
                else if (QString("access-list").startsWith(myArgs.at(1), Qt::CaseInsensitive) && QString(myArgs.at(1)).size() > 1)
                {
                    if (myArgs.at(2) == "?")
                    {
                        emit Write("  <1-199>  ACL number\n"
                                   "  WORD     ACL name\n"
                                   "  <cr>");
                    }
                    else
                    {
                        if (QRegExp("\\d{1,3}\\?").exactMatch(myArgs.at(2)))
                        {
                            if (QString(myArgs.at(2)).remove("?").toInt() > 0 &&
                                QString(myArgs.at(2)).remove("?").toInt() < 200)
                            {
                                emit Write("<1-199>  WORD");
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
                    emit Finished();
                }
                else
                {
                    if (QString("route").startsWith(myArgs.at(1), Qt::CaseInsensitive) ||
                        QString("interface").startsWith(myArgs.at(1), Qt::CaseInsensitive) ||
                        QString("access-list").startsWith(myArgs.at(1), Qt::CaseInsensitive))
                    {
                        QString str_buf = myArgsStr;
                        emit Write("% Ambiguous command: \"" + device()->cur_cmd + " " +
                                   str_buf.remove(QRegExp("\\?$")).simplified() + "\"");
                    }
                    else
                    {
                        emit Write("% Unrecognized command");
                    }
                    emit Finished();
                }
            }
            else
            {
                if (QString("ip").startsWith(myArgs.at(0), Qt::CaseInsensitive) ||
                    QString("interfaces").startsWith(myArgs.at(0), Qt::CaseInsensitive))
                {
                    QString str_buf = myArgsStr;
                    emit Write("% Ambiguous command: \"" + device()->cur_cmd + " " +
                               str_buf.remove(QRegExp("\\?$")).simplified() + "\"");
                }
                else
                {
                    emit Write("% Unrecognized command");
                }
                emit Finished();
            }
            break;
        case 4:
            if (QRegExp("^in[a-z]{,8} *F(a(s(t(E(t(h(e(r(n(e(t)?)?)?)?)?)?)?)?)?)?)? *\\d/\\d +\\?", Qt::CaseInsensitive).exactMatch(myArgsStr))
            {
                emit Write("  <cr>");
                emit Finished();
            }
            else if (QString("ip").startsWith(myArgs.at(0), Qt::CaseInsensitive) && QString(myArgs.at(0)).size()>1)
            {
                if (QString("interface").startsWith(myArgs.at(1), Qt::CaseInsensitive) && QString(myArgs.at(1)).size()>2)
                {
                    if (QString("FastEthernet").startsWith(myArgs.at(2), Qt::CaseInsensitive) && QString(myArgs.at(2)).size() > 1)
                    {
                        if (myArgs.at(3) == "?")
                        {
                            emit Write("  <0-9>  FastEthernet interface number");
                        }
                        else if (QRegExp("\\d/\\d\\?").exactMatch(myArgs.at(3)))
                        {
                            emit Write("/    -");
                        }
                        else
                        {
                            emit Write("/");
                        }
                        emit Finished();
                    }
                    else
                    {
                        emit Write("% Unrecognized command");
                        emit Finished();
                    }
                }
                else if (QString("access-list").startsWith(myArgs.at(1), Qt::CaseInsensitive) && QString(myArgs.at(1)).size() > 1)
                {
                    if (myArgs.at(3) == "?")
                    {
                        emit Write("  <cr>");
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
            }
            else
            {
                emit Write("% Unrecognized command");
                emit Finished();
            }
            break;
            case 5:
            if (QRegExp("^ip int(e(r(f(a(ce?)?)?)?)?)? *F(a(s(t(E(t(h(e(r(n(e(t)?)?)?)?)?)?)?)?)?)?)? *\\d/\\d +\\?", Qt::CaseInsensitive).exactMatch(myArgsStr))
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
    if (QString("interfaces").startsWith(myArgs.at(0), Qt::CaseInsensitive) && QString(myArgs.at(0)).size() > 1)
    {
        QRegExp rx("^in[a-z]{,8} +F(a(s(t(E(t(h(e(r(n(e(t)?)?)?)?)?)?)?)?)?)?)? *\\d/\\d$", Qt::CaseInsensitive);
        if (myArgs.size() == 1)
        {
            bool isFirst = true;
            foreach (Device::deviceInterface iface, device()->Interfaces())
            {
                if (isFirst)
                {
                    isFirst = false;
                }
                else
                {
                    str.append("\n");
                }

                str.append(iface.name);
                if (iface.state)
                {
                    str.append(" is up");
                }
                else
                {
                    str.append(" is administratively down");
                }

                QString str_up(", line protocol is down (disabled)\n");
                // Перебираем свои линки
                foreach (Device::deviceLink* link, device()->Links())
                {
                    Device* d;
                    devicePort* p;
                    // В зависимости от того на каком конце линка Мы - запоминаем данные удалённой стороны
                    if (link->firstDevice == this->device() && link->firstPort == iface.AssignedPort)
                    {
                        d = link->secondDevice;
                        p = link->secondPort;
                    }
                    else if (link->secondDevice == this->device() && link->secondPort == iface.AssignedPort)
                    {
                        d = link->firstDevice;
                        p = link->firstPort;
                    }
                    else
                    {
                        continue;
                    }
                    // Перебираем интерфейсы удалённого устройства
                    foreach (QString key1, d->Interfaces().keys())
                    {
                        // Если этот интерфейс использует тот порт к которому мы подключены, то...
                        if (d->Interfaces()[key1].AssignedPort == p)
                        {
                            // Удаленный интерфейс должен быть поднят (Cisco они такие...)
                            if (d->Interfaces()[key1].state)
                            {
                                str_up = ", line prolocol is up\n";
                                break;
                            }
                            else
                            {
                                str_up = ", line protocol is down (disabled)\n";
                                break;
                            }
                        }
                    }
                }
                str += str_up;

                QString mac = iface.hwaddr.remove(":");
                mac = mac.left(4) + "." + mac.mid(4,4) + "." + mac.right(4);
                str += "        Hardware is Lance, address is ";
                str.append(mac).append(" (bia ").append(mac).append(QString(")\n        MTU %1").arg(iface.mtu));
                str += " bytes, BW 100000 Kbit, DLY 100 usec,\n"
                       "           reliability 255/255, txload 1/255, rxload 1/255\n"
                       "        Encapsulation ARPA, loopback not set";
            }
        }
        else if (rx.exactMatch(myArgsStr))
        {
            bool ifExist = false;
            foreach (Device::deviceInterface iface, device()->Interfaces())
            {
                //                if (myArgsStr.section(" ",1).remove(" ").compare(iface.name, Qt::CaseInsensitive))
                if (myArgsStr.right(3) == iface.name.right(3))
                {
                    ifExist = true;

                    str.append(iface.name);
                    if (iface.state)
                    {
                        str.append(" is up");
                    }
                    else
                    {
                        str.append("is administratively down");
                    }
                    QString mac = iface.hwaddr.remove(":");
                    mac = mac.left(4) + "." + mac.mid(4,4) + "." + mac.right(4);
                    str += ", line protocol is up\n"
                           "        Hardware is Lance, address is ";
                    str.append(mac).append(" (bia ").append(mac).append(QString(")\n        MTU %1").arg(iface.mtu));
                    str += " bytes, BW 100000 Kbit, DLY 100 usec,\n"
                           "           reliability 255/255, txload 1/255, rxload 1/255\n"
                           "        Encapsulation ARPA, loopback not set";
                }
            }
            if (!ifExist)
            {
                str = "%Invalid interface type and number";
            }
        }
        else
        {
            emit MarkError(device()->cur_cmd + " " + myArgsStr.left(rx.matchedLength()));
        }
    }
    else if (QString("ip").startsWith(myArgs.at(0), Qt::CaseInsensitive) && QString(myArgs.at(0)).size() > 1)
    {
        QRegExp rx_ipro("ip +(ac(c(e(s(s(-(l(i(st?)?)?)?)?)?)?)?)?.*|ro(u(te?)?)?|int(e(r(f(a(ce?)?)?)?)?)?.*)", Qt::CaseInsensitive);
        if (rx_ipro.exactMatch(myArgsStr))
        {
            if (QString("route").startsWith(myArgs.at(1), Qt::CaseInsensitive))
            {
                str = "Codes: C - connected, S - static, I - IGRP, R - RIP, M - mobile, B - BGP\n"
                      "            D - EIGRP, EX - EIGRP external, O - OSPF, IA - OSPF inter area\n"
                      "            N1 - OSPF NSSA external type 1, N2 - OSPF NSSA external type 2\n"
                      "            E1 - OSPF external type 1, E2 - OSPF external type 2, E - EGP\n"
                      "            i - IS-IS, L1 - IS-IS level-1, L2 - IS-IS level-2, ia - IS-IS inter area\n"
                      "            * - candidate default, U - per-user static route, o - ODR\n"
                      "            P - periodic downloaded static route\n\n";

                // For connected route
                QString rc = "";
                // For default route
                QString rd = "";
                // For subnet separate
                QHash<QString, QString> rs;

                // Show route only if link is UP,UP and distination is reachable

                //Перебираем свои интерфейсы
                foreach (QString key, device()->Interfaces().keys())
                {
                    // Рассматриваем только в состоянии UP,UP
                    if (device()->Interfaces()[key].state && device()->Interfaces()[key].AssignedPort->busy)
                    {
                        // Перебираем свои линки
                        foreach (Device::deviceLink* link, device()->Links())
                        {
                            Device* d;
                            devicePort* p;
                            // В зависимости от того на каком конце линка Мы - запоминаем данные удалённой стороны
                            if (link->firstDevice == this->device() && link->firstPort == device()->Interfaces()[key].AssignedPort)
                            {
                                d = link->secondDevice;
                                p = link->secondPort;
                            }
                            else if (link->secondDevice == this->device() && link->secondPort == device()->Interfaces()[key].AssignedPort)
                            {
                                d = link->firstDevice;
                                p = link->firstPort;
                            }
                            else
                            {
                                continue;
                            }
                            // Перебираем интерфейсы удалённого устройства
                            foreach (QString key1, d->Interfaces().keys())
                            {
                                // Если этот интерфейс использует тот порт к которому мы подключены, то...
                                if (d->Interfaces()[key1].AssignedPort == p)
                                {
                                    // Удаленный интерфейс должен быть поднят (Cisco они такие...)
                                    if (d->Interfaces()[key1].state)
                                    {
                                        // Перебераем свою таблицу маршрутизации
                                        foreach (Device::route r, device()->routes())
                                        {
                                            // Если это маршрут через "проверенный" интерфейс, то...
                                            if (r.dev == key)
                                            {
                                                // Если это маршрут обозначенный подсетью интерфейса, то печатаем его
                                                if (r.scope == "link")
                                                {
                                                    rc.append("C   ").append(r.net.toString()).append(QString("/%1").arg(
                                                            r.prefix)).append(" is directly connected, ").append(r.dev).append("\n");
                                                }
                                                else if (r.gw.isInSubnet(device()->Interfaces()[key].address.ip(),
                                                                         device()->Interfaces()[key].address.prefixLength()))
                                                {
                                                    if (r.prefix == 32)
                                                    {
                                                        rs.insertMulti(r.net.toString().section(".",0,0).append(".0.0.0"),
                                                                       r.net.toString().append("    [1/0] via ").append(
                                                                               r.gw.toString()));
                                                    }
                                                    else if (r.net == QHostAddress::Any && r.prefix == 0)
                                                    {
                                                        rd.append("S*  0.0.0.0/0 [1/0] via ").append(r.gw.toString());
                                                    }
                                                    else
                                                    {
                                                        rs.insertMulti(r.net.toString().section(".",0,0).append(".0.0.0"),
                                                                       r.net.toString().append(QString("/%1 [1/0] via ").arg(
                                                                               r.prefix)).append(r.gw.toString()));
                                                    }
                                                }
                                            }
                                        }
                                    }
                                    break;
                                }
                            }
                        }
                    }
                }

                if (!rd.isEmpty())
                {
                    str.append("Gateway of last resort is ").append(rd.section("via ",1)).append(" to network 0.0.0.0\n\n");
                }
                // Выводим всё это добро по подсетям и по маскам
                foreach(QString key, rs.uniqueKeys())
                {
                    QHash<QString, int> masks;
                    QHash<QString, QString> subnets;
                    foreach(QString s, rs.values(key))
                    {
                        QString m("");
                        if (s.split("/").size() == 2)
                        {
                            m = QString(s.split("/").at(1)).section(" ", 0, 0);
                        }
                        else
                        {
                            m = QString("32");
                        }

                        QString _str("");
                        if (subnets.value(s.section(QRegExp("[/ ]"),0,0).append("/").append(m)) != "")
                        {
                            _str.append("                                [").append(s.section("[",1)).append("\n");
                        }
                        else
                        {
                            _str.append("S             ").append(s).append("\n");
                        }
                        masks.insertMulti(m, 1);
                        subnets.insertMulti(s.section(QRegExp("[/ ]"),0,0).append("/").append(m), _str);
                    }

                    QString str_cur("");
                    quint32 common = 0;

                    foreach(QString uk, subnets.uniqueKeys())
                    {
                        QString SS("");
                        QString notSS("");
                        foreach(QString _s, subnets.values(uk))
                        {
                            if (_s.startsWith("S"))
                            {
                                SS = _s;
                            }
                            else
                            {
                                notSS.append(_s);
                            }
                        }
                        str_cur.append(SS).append(notSS);

                        // Кривой алгоритм, хз как реально происходит расчёт этой "маски" у Cisco
                        if (common == 0)
                        {
                            common = QHostAddress(uk.section("/",0,0)).toIPv4Address();
                        }
                        else
                        {
                            common = common | QHostAddress(uk.section("/",0,0)).toIPv4Address();
                        }
                    }
                    // См. предыдущий комент
                    quint8 spref = QString::number(common, 2).remove(QRegExp("0{,32}$")).size();
                    // Поэтому будет всегда /8
                    spref = 8;

                    if (masks.uniqueKeys().size() > 1)
                    {
                        str_cur.insert(0, QString("     ").append(key).append(QString("/%1 is variably subnetted,"
                                                                                      " %2 subnets, %3 masks").arg(
                                                                                              spref).arg(subnets.uniqueKeys().size()).arg(masks.uniqueKeys().size())).append("\n"));
                    }
                    else
                    {
                        str_cur.insert(0, QString("     ").append(key).append(QString("/%1 is subnetted, %2 subnets").arg(
                                spref).arg(subnets.uniqueKeys().size())).append("\n"));
                    }
                    str.append(str_cur);
                }

                str.append(rc);
                str.append(rd);
            }
            else if (QString("interface").startsWith(myArgs.at(1), Qt::CaseInsensitive))
            {
                QRegExp rx("^ip +int(e(r(f(a(ce?)?)?)?)?)?( +F(a(s(t(E(t(h(e(r(n(e(t)?)?)?)?)?)?)?)?)?)?)? *\\d/\\d$)?$", Qt::CaseInsensitive);
                if (rx.exactMatch(myArgsStr))
                {
                    if (QRegExp("^ip +int(e(r(f(a(ce?)?)?)?)?)?$", Qt::CaseInsensitive).exactMatch(myArgsStr))
                    {
                        str = cmdShow_ip_int();
                    }
                    else
                    {
                        if (myArgs.size() == 3)
                        {
                            str = cmdShow_ip_int(myArgs.at(2));
                        }
                        else
                        {
                            str = cmdShow_ip_int(myArgs.at(3));
                        }
                    }
                }
                else
                {
                    emit MarkError(device()->cur_cmd + " " + myArgsStr.left(rx.matchedLength()));
                }
            }
            else if (QString("access-list").startsWith(myArgs.at(1), Qt::CaseInsensitive))
            {
                QRegExp rx("ip +ac(c(e(s(s(-(l(i(st?)?)?)?)?)?)?)?)?", Qt::CaseInsensitive);
                if (rx.exactMatch(myArgsStr))
                {
                    router* r = static_cast<router*>(device());
                    foreach (router::ACL a, r->acls())
                    {
                        str.append("\n").append(a.type).append(" IP access list ").append(a.name);
                        foreach (router::ACL_rule ru, a.rules)
                        {
                            str.append("\n   ").append(ru.action).append(" ").append(ru.proto).append(" ").append(
                                    ru.src.toString()).append(" ").append(ru.dst.toString());
                            if (!ru.type.isEmpty())
                            {
                                str.append(" ").append(ru.type).append(QString(" %1").arg(ru.port));
                            }
                        }
                    }
                    if (str.isEmpty())
                    {
                        emit Finished();
                    }
                    else
                    {
                        str.remove(QRegExp("^\\n"));
                    }
                }
                else
                {
                    emit MarkError(device()->cur_cmd + " " + myArgsStr.left(rx.matchedLength()));
                }
            }
        }
        else
        {
            emit MarkError(device()->cur_cmd + " " + myArgsStr.left(rx_ipro.matchedLength()));
        }
    }
    else if (myArgs.at(0) == "error")
    {
        str = "% Incomplete command.";
    }
    else
    {
        rx_0.exactMatch(myArgs.at(0));
        emit MarkError(device()->cur_cmd + " " + QString(myArgs.at(0)).left(rx_0.matchedLength()));
    }

    if (!str.isEmpty())
    {
        emit Write(str);
        emit Finished();
    }
}

QString RouterShowCommand::cmdShow_ip_int(const QString& if_name) const
{
    QString str("");
    bool isExist = false;
    bool isFirst = true;
    foreach (Device::deviceInterface iface, device()->Interfaces())
    {
        if (!if_name.isEmpty())
        {
            // Work only if max iface number is 9 - consist from one digit
            if (if_name.right(3) != iface.name.right(3))
            {
                continue;
            }
            else
            {
                isExist = true;
            }
        }

        if (isFirst)
        {
            isFirst = false;
        }
        else
        {
            str.append("\n");
        }

        str.append(iface.name);
        if (iface.state)
        {
            str.append(" is up");
        }
        else
        {
            str.append(" is administratively down");
        }

        QString str_up(", line protocol is down (disabled)\n");
        // Перебираем свои линки
        foreach (Device::deviceLink* link, device()->Links())
        {
            Device* d;
            devicePort* p;
            // В зависимости от того на каком конце линка Мы - запоминаем данные удалённой стороны
            if (link->firstDevice == this->device() && link->firstPort == iface.AssignedPort)
            {
                d = link->secondDevice;
                p = link->secondPort;
            }
            else if (link->secondDevice == this->device() && link->secondPort == iface.AssignedPort)
            {
                d = link->firstDevice;
                p = link->firstPort;
            }
            else
            {
                continue;
            }
            // Перебираем интерфейсы удалённого устройства
            foreach (QString key1, d->Interfaces().keys())
            {
                // Если этот интерфейс использует тот порт к которому мы подключены, то...
                if (d->Interfaces()[key1].AssignedPort == p)
                {
                    // Удаленный интерфейс должен быть поднят (Cisco они такие...)
                    if (d->Interfaces()[key1].state)
                    {
                        str_up = ", line prolocol is up\n";
                        break;
                    }
                    else
                    {
                        str_up = ", line protocol is down (disabled)\n";
                        break;
                    }
                }
            }
        }
        str += str_up;


        str.append("  Internet address is ").append(iface.address.ip().toString()).append(
                QString("/%1\n").arg(iface.address.prefixLength()));
        str.append("  Broadcast address is ").append(iface.address.broadcast().toString()).append("\n");
        str.append("  Address determined by setup command\n");
        str.append(QString("  MTU is %1").arg(iface.mtu));
    }

    if (!if_name.isEmpty() && !isExist)
    {
        str = "%Invalid interface type and number";
    }

    return str;
}
