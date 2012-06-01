#ifndef DIAGRAMITEM_H
#define DIAGRAMITEM_H

#include <QGraphicsPixmapItem>
#include <QList>

class QPixmap;
class QGraphicsItem;
class QGraphicsItemGroup;
class QGraphicsTextItem;
class QGraphicsScene;
class QTextEdit;
class QGraphicsSceneMouseEvent;
class QMenu;
class QGraphicsSceneContextMenuEvent;
class QStyleOptionGraphicsItem;
class QWidget;

class Link;
class Device;
class PC;

class DiagramItem : public QObject, public QGraphicsPixmapItem
{
    Q_OBJECT

    friend class MainWindow;

    public:
        enum { Type = UserType + 15 };
        enum DiagramType {PC_item, router_item, commutator_item};

        DiagramItem(DiagramType diagramType, QMenu* contextMenu,
            QGraphicsItem* parent = 0, QGraphicsScene* scene = 0);

        void removeDevice();
        void removeLink(Link* link);
        void removeLinks();
        DiagramType diagramType() const
            { return myDiagramType; }
        void addLink(Link* link);
        int type() const
            { return Type;}
        void AssignDevice(Device* dev);
        bool CheckEmployment();
        QGraphicsItemGroup* group;

    protected:
        void contextMenuEvent(QGraphicsSceneContextMenuEvent* event);
        QVariant itemChange(GraphicsItemChange change, const QVariant& value);
        void mouseReleaseEvent(QGraphicsSceneMouseEvent* mouseEvent);
        void mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event);

    private:
        DiagramType myDiagramType;
        QMenu* myContextMenu;
        QList<Link*> links;
        Device* device;
        QGraphicsTextItem* caption;
        QGraphicsTextItem* text;

    public slots:
        void updateInfo();

    signals:
        void itemSelected(DiagramItem*);
        void PropertiesRequest(DiagramItem*);
};

#endif
