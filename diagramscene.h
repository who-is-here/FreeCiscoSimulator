#ifndef DIAGRAMSCENE_H
#define DIAGRAMSCENE_H

#include <QGraphicsScene>
#include "diagramitem.h"

class QGraphicsSceneMouseEvent;
class QMenu;
class QPointF;
class QGraphicsLineItem;
class QFont;
class QGraphicsTextItem;
class QColor;

class DiagramScene : public QGraphicsScene
{
    Q_OBJECT

public:
    enum Mode { InsertItem, InsertLine, MoveItem };

    DiagramScene(QMenu* itemMenu, QObject* parent = 0);

    //QList<DiagramItem*> Devices;

public slots:
    void setMode(Mode mode);
    void setItemType(DiagramItem::DiagramType type);

signals:
    void itemInserted(DiagramItem* item);
    void itemSelected(DiagramItem* item);
    void linkInserted(DiagramItem* firstItem, DiagramItem* secondItem, Link* link);

    void propertiesRequest(DiagramItem* item);

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent* mouseEvent);
    void mouseMoveEvent(QGraphicsSceneMouseEvent* mouseEvent);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* mouseEvent);

private:
    bool isItemChange(int type);

    DiagramItem::DiagramType myItemType;
    QMenu* myItemMenu;
    Mode myMode;
    bool leftButtonDown;
    QGraphicsLineItem* line;
};

#endif
