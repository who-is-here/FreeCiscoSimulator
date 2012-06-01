#include "ipcommand.h"

#include "Device.h"

PCIpCommand::PCIpCommand(Device *device):
        DeviceCommand("ip", device)
{
}

void PCIpCommand::exec(const QString& args)
{
    QStringList argsInternal = args.split(" ");
    QString str = "";
    if (QString(argsInternal.at(0)).isEmpty())
    {
        argsInternal.replace(0, "error");
    }
    if (QString("address").startsWith(argsInternal.at(0)))
    {

        if (argsInternal.size() == 1)
        {
            argsInternal.append("s");
        }
        if (QString("show").startsWith(argsInternal.at(1)))
        {
            QHash<QString, Device::deviceInterface> Interfaces = device()->Interfaces();
            str = "1: lo: <LOOPBACK,UP,LOWER_UP> mtu 16436 qdisc noqueue state UNKNOWN\n"
                  "      link/loopback 00:00:00:00:00:00 brd 00:00:00:00:00:00\n"
                  "      inet 127.0.0.1/8 scope host lo\n"
                  "      inet6 ::1/128 scope host\n"
                  "      valid_lft forever preferred_lft forever";
            unsigned int i = 1;
            foreach (QString key, Interfaces.keys())
            {
                QString state = "DOWN";
                if (Interfaces[key].state)
                {
                    state = "UP";
                }
                str.append(QString("\n%1: ").arg(++i).append(Interfaces[key].name).append(
                        ": <BROADCAST,MULTICAST,UP,LOWER_UP> mtu %1 qdisc pfifo_fast state ").arg(
                        Interfaces[key].mtu).append(state).append(" qlen 1000\n      link/ether ").append(
                                Interfaces[key].hwaddr).append(" brd ff:ff:ff:ff:ff:ff\n      inet ").append(
                                Interfaces[key].address.ip().toString()).append(" brd ").append(
                                        Interfaces[key].address.broadcast().toString()).append(" scope global ").append(
                                                Interfaces[key].name).append("\n         valid_lft"
                                                                             " forever preferred_lft forever"));
            }
        }
        else if (QString("delete").startsWith(argsInternal.at(1)))
        {
            if (argsInternal.size() < 4)
            {
                str = "Not enough information: \"dev\" argument is required.";
            }
            else
            {
                if (argsInternal.at(3) == "dev")
                {
                    if (argsInternal.size() < 5)
                    {
                        str = "Command line is not complete. Try option \"help\"";
                    }
                    else
                    {
                        QHash<QString, Device::deviceInterface> Interfaces = device()->Interfaces();

                        bool dev_exist = false;
                        foreach (QString key, Interfaces.keys())
                        {
                            if (Interfaces[key].name == argsInternal.at(4))
                            {
                                dev_exist = true;
                                if (argsInternal.at(2) == Interfaces[key].address.ip().toString().append(
                                        "/%1").arg(Interfaces[key].address.prefixLength()))
                                {
                                    ConfigureInterface(argsInternal.at(4),"");
                                }
                                else
                                {
                                    str = "Cannot assign requested address";
                                }
                            }
                        }
                        if (!dev_exist)
                        {
                            str = QString("Cannot find device \"").append(argsInternal.at(4)).append("\"");
                        }
                    }
                }
                else
                {
                    str = "Not enough information: \"dev\" argument is required.";
                }
            }
        }
        else if (QString("add").startsWith(argsInternal.at(1)))
        {
            if (argsInternal.size() < 4)
            {
                str = "Not enough information: \"dev\" argument is required.";
            }
            else
            {
                if (argsInternal.at(3) == "dev")
                {
                    if (argsInternal.size() < 5)
                    {
                        str = "Command line is not complete. Try option \"help\"";
                    }
                    else
                    {
                        bool dev_exist = false;
                        QHash<QString, Device::deviceInterface> Interfaces = device()->Interfaces();

                        foreach (QString key, Interfaces.keys())
                        {
                            if (Interfaces[key].name == argsInternal.at(4))
                            {
                                dev_exist = true;
                                bool isPrefix;
                                int Prefix = QString(argsInternal.at(2)).section("/",1).toInt(&isPrefix,10);
                                if (!isPrefix && Prefix == 0 && QString(argsInternal.at(2)).section("/",1).isEmpty() &&
                                    !QString(argsInternal.at(2)).endsWith("/"))
                                {
                                    isPrefix = true;
                                    Prefix = 32;
                                }
                                QString _ip = argsInternal.at(2).section("/",0,0);
                                if (!QHostAddress(_ip).isNull() && isPrefix && Prefix < 33 && Prefix >= 0)
                                {
//                          ***** It's strange but is legal situation *****
//                                  if (QHostAddress(_ip) == Device::CalculateSubNet(_ip, Prefix) || QHostAddress(_ip) == Device::CalculateBroadCast(_ip, Prefix))
//                          {
//                              str = "FAIL";
//                          }
//                          else
//                          {
                                    str = "In this version of network simulator each interface can have only one ip address.\n"
                                          "Old address was overwritten.";
                                    ConfigureInterface(argsInternal.at(4),QString(argsInternal.at(2)).section("/",0,0),Prefix);
//                                      }
                                }
                                else
                                {
                                    str = QString("Error: an inet prefix is expected rather than \"").append(argsInternal.at(2)).append("\".");
                                }
                            }
                        }
                        if (!dev_exist)
                        {
                            str = QString("Cannot find device \"").append(argsInternal.at(4)).append("\"");
                        }
                    }
                }
                else
                {
                    str = "Not enough information: \"dev\" argument is required.";
                }
            }
        }
        else if (QString("help").startsWith(argsInternal.at(1)))
        {
            str = "Usage: ip addr {add|change|replace} IFADDR dev STRING [ LIFETIME ]\n"
                  "                                                                                    [ CONFFLAG-LIST]\n"
                  "ip addr del IFADDR dev STRING\n"
                  "ip addr {show|flush} [ dev STRING ] [ scope SCOPE-ID ]\n"
                  "                        [ to PREFIX ] [ FLAG-LIST ] [ label PATTERN ]\n"
                  "IFADDR := PREFIX | ADDR peer PREFIX\n"
                  "                [ broadcast ADDR ] [ anycast ADDR ]\n"
                  "                [ label STRING ] [ scope SCOPE-ID ]\n"
                  "SCOPE-ID := [ host | link | global | NUMBER ]\n"
                  "FLAG-LIST := [ FLAG-LIST ] FLAG\n"
                  "FLAG  := [ permanent | dynamic | secondary | primary |\n"
                  "                tentative | deprecated | CONFFLAG-LIST ]\n"
                  "CONFFLAG-LIST := [ CONFFLAG-LIST ] CONFFLAG\n"
                  "CONFFLAG  := [ home | nodad ]\n"
                  "LIFETIME := [ valid_lft LFT ] [ preferred_lft LFT ]\n"
                  "LFT := forever | SECONDS";
        }
        else
        {
            str = QString("Command \"").append(argsInternal.at(1)).append("\" is unknown, try \"ip addr help\".");
        }
    }
    else if (QString("route").startsWith(argsInternal.at(0)))
    {
        if (argsInternal.size() == 1)
        {
            argsInternal.append("s");
        }
        if (QString("show").startsWith(argsInternal.at(1)))
        {
            QList<Device::route> routes = device()->routes();

            foreach (const Device::route& r, routes)
            {
                if (!str.isEmpty())
                {
                    str.append("\n");
                }
                if (r.net.toString().append(QString("/%1").arg(r.prefix)) == QString("0.0.0.0/0"))
                {
                    str.append("default");
                }
                else
                {
                    str.append(r.net.toString()).append(QString("/%1").arg(r.prefix));
                }
                if (r.gw.toString() != "")
                {
                    str.append(" via ").append(r.gw.toString());
                }
                str.append(" dev ").append(r.dev);
                if (!r.proto.isEmpty())
                {
                    str.append(" proto ").append(r.proto);
                }
                if (r.scope != "global")
                {
                    str.append(" scope ").append(r.scope);
                }
                if (r.src != QHostAddress::Null && r.scope == "link")
                {
                    str.append(" src ").append(r.src.toString());
                }
            }
        }
        else if (QString("add").startsWith(argsInternal.at(1)) || QString("delete").startsWith(argsInternal.at(1)))
        {
            if (argsInternal.size() < 4)
            {
                str = "No such device";
            }
            else
            {
                bool isPrefix;
                int Prefix = QString(argsInternal.at(2)).section("/",1).toInt(&isPrefix,10);
                if (!isPrefix && Prefix == 0 && QString(argsInternal.at(2)).section("/",1).isEmpty() &&
                    !QString(argsInternal.at(2)).endsWith("/"))
                {
                    isPrefix = true;
                    Prefix = 32;
                }
                if (!QHostAddress(argsInternal.at(2).section("/",0,0)).isNull() && isPrefix && Prefix < 33 && Prefix >= 0)
                {
                    if (argsInternal.at(3) == "dev")
                    {
                        if (argsInternal.size() < 5)
                        {
                            str = "Command line is not complete. Try option \"help\"";
                        }
                        else
                        {
                            bool dev_exist = false;
                            QHash<QString, Device::deviceInterface> Interfaces = device()->Interfaces();

                            foreach (QString key, Interfaces.keys())
                            {
                                if (Interfaces[key].name == argsInternal.at(4))
                                {
                                    dev_exist = true;
                                    if (QString("add").startsWith(argsInternal.at(1)))
                                    {
                                        addRoute(argsInternal.at(4), QString(argsInternal.at(2)).section("/",0,0),Prefix);
                                    }
                                    else
                                    {
                                        delRoute(argsInternal.at(4), CalculateSubNet(QString(argsInternal.at(2)).section(
                                                "/",0,0),Prefix).toString(), Prefix);
                                    }
                                }
                            }
                            if (!dev_exist)
                            {
                                str = QString("Cannot find device \"").append(argsInternal.at(4)).append("\"");
                            }
                        }
                    }
                    else if (argsInternal.at(3) == "via")
                    {
                        QList<Device::route> routes = device()->routes();

                        if (QHostAddress(argsInternal.at(4)).isNull())
                        {
                            str = QString("Error: an inet address is expected rather than \"").append(
                                    argsInternal.at(4)).append("\".");
                        }
                        else
                        {

                            bool isReachable = false;
                            foreach (const Device::route& r, routes)
                            {
                                if (QHostAddress(argsInternal.at(4)).isInSubnet(r.net,r.prefix))
                                {
                                    isReachable = true;
                                    if (QString("add").startsWith(argsInternal.at(1)))
                                    {
                                        addRoute(r.dev, argsInternal.at(2).section("/",0,0), Prefix, argsInternal.at(4));
                                        break;
                                    }
                                    else
                                    {
                                        delRoute(r.dev, argsInternal.at(2).section("/",0,0), Prefix, argsInternal.at(4));
                                        break;
                                    }
                                }
                            }
                            if (!isReachable)
                            {
                                str = "No such process";
                            }
                        }
                    }
                    else
                    {
                        str = QString("Error: either \"to\" is duplicate, or \"").append(argsInternal.at(3)).append("\" is a garbage.");
                    }
                }
                else
                {
                    str = QString("Error: an inet prefix is expected rather than \"").append(argsInternal.at(2)).append("\".");
                }
            }
        }
        else if (QString("help").startsWith(argsInternal.at(1)))
        {
            str = "Usage: ip route { list | flush } SELECTOR\n"
                  "            ip route get ADDRESS [ from ADDRESS iif STRING ]\n"
                  "                                                [ oif STRING ]  [ tos TOS ]\n"
                  "            ip route { add | del | change | append | replace | monitor } ROUTE\n"
                  "SELECTOR := [ root PREFIX ] [ match PREFIX ] [ exact PREFIX ]\n"
                  "                       [ table TABLE_ID ] [ proto RTPROTO ]\n"
                  "                       [ type TYPE ] [ scope SCOPE ]\n"
                  "ROUTE := NODE_SPEC [ INFO_SPEC ]\n"
                  "NODE_SPEC := [ TYPE ] PREFIX [ tos TOS ]\n"
                  "                        [ table TABLE_ID ] [ proto RTPROTO ]\n"
                  "                        [ scope SCOPE ] [ metric METRIC ]\n"
                  "INFO_SPEC := NH OPTIONS FLAGS [ nexthop NH ]...\n"
                  "NH := [ via ADDRESS ] [ dev STRING ] [ weight NUMBER ] NHFLAGS\n"
                  "OPTIONS := FLAGS [ mtu NUMBER ] [ advmss NUMBER ]\n"
                  "                      [ rtt TIME ] [ rttvar TIME ] [reordering NUMBER ]\n"
                  "                      [ window NUMBER] [ cwnd NUMBER ] [ initcwnd NUMBER ]\n"
                  "                      [ ssthresh NUMBER ] [ realms REALM ] [ src ADDRESS ]\n"
                  "                      [ rto_min TIME ] [ hoplimit NUMBER ] \n"
                  "TYPE := [ unicast | local | broadcast | multicast | throw |\n"
                  "                     unreachable | prohibit | blackhole | nat ]\n"
                  "TABLE_ID := [ local | main | default | all | NUMBER ]\n"
                  "SCOPE := [ host | link | global | NUMBER ]\n"
                  "FLAGS := [ equalize ]\n"
                  "MP_ALGO := { rr | drr | random | wrandom }\n"
                  "NHFLAGS := [ onlink | pervasive ]\n"
                  "RTPROTO := [ kernel | boot | static | NUMBER ]\n"
                  "TIME := NUMBER[s|ms|us|ns|j]";
        }
        else
        {
            str = QString("Command \"").append(argsInternal.at(1)).append("\" is unknown, try \"ip route help\".");
        }
    }
    else if (QString("rule").startsWith(argsInternal.at(0)))
    {
        str = "rules not implemented";
    }
    else if (QString("link").startsWith(argsInternal.at(0)))
    {
        if (QRegExp("^l(i(nk?)?)? s(et?)? .*").exactMatch(args))
        {
            bool pass = true;
            QString if_name = "";
            QString if_action = "";
            if (QRegExp("^l(i(nk?)?)? s(et?)? dev eth\\d (up|down)$").exactMatch(args))
            {
                if_name = argsInternal.at(3);
                if_action = argsInternal.at(4);
            }
            else if(QRegExp("^l(i(nk?)?)? s(et?)? eth\\d (up|down)$").exactMatch(args))
            {
                if_name = argsInternal.at(2);
                if_action = argsInternal.at(3);
            }
            else
            {
                pass = false;
                str = "Error";
            }
            if (pass)
            {
                bool dev_exist = false;
                foreach (QString key, device()->Interfaces().keys())
                {
                    if (if_name == key)
                    {
                        dev_exist = true;
                    }
                }
                if (!dev_exist)
                {
                    str = "Cannot find device \"" + if_name + "\"";
                }
                else
                {
                    if (if_action == "up")
                    {
                        device()->SetInterfaceState(if_name, true);
                    }
                    else
                    {
                        device()->SetInterfaceState(if_name, false);
                    }
                }
            }
        }
        else if (QRegExp("^l(i(nk?)?)? h(e(lp?)?)?.*").exactMatch(args))
        {
            str = "Usage: ip link add link DEV [ name ] NAME\n"
                  "                  [ txqueuelen PACKETS ]\n"
                  "                  [ address LLADDR ]\n"
                  "                  [ broadcast LLADDR ]\n"
                  "                  [ mtu MTU ]\n"
                  "                  type TYPE [ ARGS ]\n"
                  "      ip link delete DEV type TYPE [ ARGS ]\n"
                  "\n"
                  "      ip link set DEVICE [ { up | down } ]\n"
                  "                         [ arp { on | off } ]\n"
                  "                         [ dynamic { on | off } ]\n"
                  "                         [ multicast { on | off } ]\n"
                  "                         [ allmulticast { on | off } ]\n"
                  "                         [ promisc { on | off } ]\n"
                  "                         [ trailers { on | off } ]\n"
                  "                         [ txqueuelen PACKETS ]\n"
                  "                         [ name NEWNAME ]\n"
                  "                         [ address LLADDR ]\n"
                  "                         [ broadcast LLADDR ]\n"
                  "                         [ mtu MTU ]\n"
                  "                         [ netns PID ]\n"
                  "                         [ alias NAME ]\n"
                  "      ip link show [ DEVICE ]\n"
                  "\n"
                  "TYPE := { vlan | veth | vcan | dummy | ifb | macvlan | can }";
        }
        else
        {
            str = "Command \"" + argsInternal.at(1) + "\" is unknown, try \"ip link help\".";
        }
    }
    else
        str = "Usage: ip [ OPTIONS ] OBJECT { COMMAND | help }\n"
              "    ip [ -force ] -batch filename\n"
              "where  OBJECT := { link | addr | addrlabel | route | rule | neigh | ntable |\n"
              "                tunnel | maddr | mroute | monitor | xfrm }\n"
              "    OPTIONS := { -V[ersion] | -s[tatistics] | -d[etails] | -r[esolve] |\n"
              "                 -f[amily] { inet | inet6 | ipx | dnet | link } |\n"
              "                 -o[neline] | -t[imestamp] | -b[atch] [filename] |\n"
              "                 -rc[vbuf] [size]}";
    emit Write(str);
    emit Finished();
}
