#include <QtGui>

#include "diagramitem.h"
#include "link.h"
#include "PC.h"
#include "router.h"
#include "deviceport.h"

DiagramItem::DiagramItem(DiagramType diagramType, QMenu *contextMenu,
             QGraphicsItem *parent, QGraphicsScene *scene)
                 : QGraphicsPixmapItem (parent, scene)
{
    myDiagramType = diagramType;
    myContextMenu = contextMenu;
    device = NULL;

    QPixmap image;
    switch (myDiagramType) {
        case PC_item:
            image.load(":/images/PC.png");
            this->device = new PC();
            break;
        case router_item:
            image.load(":/images/router.png");
            this->device = new router();
            break;
        case commutator_item:
            image.load(":/images/commutator.png");
            this->device = new PC();
            break;
        default:
            ;
    }
    caption = new QGraphicsTextItem(this, scene);
    caption->setHtml("<u>" + this->device->GetDeviceID() + "</u>");
    QString str = "";
    foreach (QString buf, this->device->GetInfo().ipAddresses)
        str.append(buf.append("\n"));
    text = new QGraphicsTextItem(str, this, scene);

    caption->setPos(-16, -16);
    text->setPos(-16, 25);

    setPixmap(image);

    setFlag(QGraphicsItem::ItemIsMovable, true);
    setFlag(QGraphicsItem::ItemIsSelectable, true);
    setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);

    connect(this->device, SIGNAL(PropertiesChnaged()), this, SLOT(updateInfo()));
}

void DiagramItem::removeLink(Link *link)
{
    int index = links.indexOf(link);

    if (index != -1)
        links.removeAt(index);
}

void DiagramItem::removeLinks()
{
    foreach (Link *link, links) {
        link->startItem()->removeLink(link);
        link->endItem()->removeLink(link);
        scene()->removeItem(link);
        delete link;
    }
}

void DiagramItem::addLink(Link *link)
{
    links.append(link);
}

void DiagramItem::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    scene()->clearSelection();
    setSelected(true);
    myContextMenu->exec(event->screenPos());
}

QVariant DiagramItem::itemChange(GraphicsItemChange change,
                     const QVariant &value)
{
    if (change == QGraphicsItem::ItemPositionChange) {
        foreach (Link *link, links) {
            link->updatePosition();
        }
    }

    return value;
}

void DiagramItem::removeDevice()
{
    delete device;
    device = NULL;
}

void DiagramItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    emit itemSelected(this);
    QGraphicsPixmapItem::mouseReleaseEvent(mouseEvent);
}
void DiagramItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    emit PropertiesRequest(this);
    QGraphicsPixmapItem::mouseDoubleClickEvent(event);
}

bool DiagramItem::CheckEmployment()
{
    bool isBusy = true;
    foreach(devicePort *port, device->Ports())
        if (!port->busy)
        {
            isBusy = false;
            break;
        }
    if (isBusy)
        QMessageBox::critical(NULL, tr("Error!"), tr("There is no free ports in ")+device->GetDeviceID()+tr(" device!"));
    return isBusy;
}

void DiagramItem::updateInfo()
{
    QString str = "";
    foreach (QString buf, this->device->GetInfo().ipAddresses)
    {
        str.append(buf.append("<br/>"));
    }
    this->text->setHtml(str);

    this->caption->setHtml("<u>" + this->device->GetDeviceID() + "</u>");
}

void DiagramItem::AssignDevice(Device *dev)
{
    removeDevice();
    device = dev;
    connect(this->device, SIGNAL(PropertiesChnaged()), this, SLOT(updateInfo()));
}
