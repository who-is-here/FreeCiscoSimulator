#include <QtGui>
#include "link.h"

Link::Link(DiagramItem *startItem, DiagramItem *endItem,
         QGraphicsItem *parent, QGraphicsScene *scene)
    : QGraphicsLineItem(parent, scene)
{
    myStartItem = startItem;
    myEndItem = endItem;
    setFlag(QGraphicsItem::ItemIsSelectable, true);
    myColor = Qt::black;
    setPen(QPen(myColor, 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
}

void Link::updatePosition()
{
    QLineF line(mapFromItem(myStartItem, 0, 0), mapFromItem(myEndItem, 0, 0));
    setLine(line);
}

void Link::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    if (myStartItem->collidesWithItem(myEndItem))
        return;

    QPen myPen = pen();
    myPen.setColor(myColor);
    painter->setPen(myPen);
    painter->setBrush(myColor);

    QPointF startPoint(myStartItem->pos().x() + myStartItem->pixmap().height()/2, myStartItem->pos().y() + myStartItem->pixmap().width()/2);
    QPointF endPoint(myEndItem->pos().x() + myEndItem->pixmap().height()/2, myEndItem->pos().y() + myEndItem->pixmap().width()/2);

    setLine(QLineF(startPoint, endPoint));
    painter->drawLine(line());
}
