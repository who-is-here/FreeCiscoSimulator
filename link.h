#ifndef LINK_H
#define LINK_H

#include <QGraphicsLineItem>

#include "diagramitem.h"
#include "Device.h"

class QGraphicsLineItem;
class QGraphicsScene;
class QGraphicsSceneMouseEvent;

class Link : public QGraphicsLineItem
{
public:
    enum { Type = UserType + 4 };

    Link(DiagramItem* startItem, DiagramItem* endItem,
      QGraphicsItem* parent = 0, QGraphicsScene* scene = 0);

    int type() const
        { return Type; }
    DiagramItem* startItem() const
        { return myStartItem; }
    DiagramItem* endItem() const
        { return myEndItem; }
    Device::deviceLink* link() const
        { return myLink; }
    void AssignLink(Device::deviceLink* link)
        { myLink = link; }

    void updatePosition();

protected:
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option,
               QWidget* widget = 0);

private:
    DiagramItem* myStartItem;
    DiagramItem* myEndItem;
    Device::deviceLink *myLink;
    QColor myColor;
};

#endif
