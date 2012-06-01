#include <QtGui>

#include "diagramscene.h"
#include "link.h"

DiagramScene::DiagramScene(QMenu *itemMenu, QObject *parent)
    : QGraphicsScene(parent)
{
    myItemMenu = itemMenu;
    myMode = MoveItem;
    myItemType = DiagramItem::PC_item;
    line = 0;
}

void DiagramScene::setMode(Mode mode)
{
    myMode = mode;
}

void DiagramScene::setItemType(DiagramItem::DiagramType type)
{
    myItemType = type;
}

void DiagramScene::mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    if (mouseEvent->button() != Qt::LeftButton)
        return;

    DiagramItem *item;
    switch (myMode) {
        case InsertItem:
            item = new DiagramItem(myItemType, myItemMenu);
            addItem(item);
            connect(item, SIGNAL(itemSelected(DiagramItem*)), this, SIGNAL(itemSelected(DiagramItem*)));
            item->setPos(mouseEvent->scenePos());
            emit itemInserted(item);
            break;
        case InsertLine:
            line = new QGraphicsLineItem(QLineF(mouseEvent->scenePos(),
                                        mouseEvent->scenePos()));
            addItem(line);
            break;
    default:
        ;
    }
    QGraphicsScene::mousePressEvent(mouseEvent);
}

void DiagramScene::mouseMoveEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    if (myMode == InsertLine && line != 0) {
        QLineF newLine(line->line().p1(), mouseEvent->scenePos());
        line->setLine(newLine);
    } else if (myMode == MoveItem) {
        QGraphicsScene::mouseMoveEvent(mouseEvent);
    } else if (myMode == InsertItem)
        return;
}

void DiagramScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    if (line != 0 && myMode == InsertLine)
    {
        QList<QGraphicsItem *> firstItems = items(line->line().p1());
        if (firstItems.count() && firstItems.first() == line)
            firstItems.removeFirst();
        QList<QGraphicsItem *> secondItems = items(line->line().p2());
        if (secondItems.count() && secondItems.first() == line)
            secondItems.removeFirst();

        removeItem(line);
        delete line;

        if (firstItems.count() > 0 && secondItems.count() > 0 &&
            firstItems.first()->type() == DiagramItem::Type &&
            secondItems.first()->type() == DiagramItem::Type &&
            firstItems.first() != secondItems.first()) {
            DiagramItem *firstItem = qgraphicsitem_cast<DiagramItem *>(firstItems.first());
            DiagramItem *secondItem = qgraphicsitem_cast<DiagramItem *>(secondItems.first());

            if (firstItem->CheckEmployment())
            {
                line = 0;
                return;
            }
            if (secondItem->CheckEmployment())
            {
                line = 0;
                return;
            }

            Link *link = new Link(firstItem, secondItem);
            firstItem->addLink(link);
            secondItem->addLink(link);
            link->setZValue(-1000.0);
            addItem(link);
            link->updatePosition();
            emit linkInserted(firstItem, secondItem, link);
        }
        myMode = MoveItem;
    }
    line = 0;
    QGraphicsScene::mouseReleaseEvent(mouseEvent);
}

bool DiagramScene::isItemChange(int type)
{
    foreach (QGraphicsItem *item, selectedItems()) {
        if (item->type() == type)
            return true;
    }
    return false;
}
