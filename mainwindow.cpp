#include <QtGui>
#include <QLabel>

#include "mainwindow.h"
#include "diagramitem.h"
#include "diagramscene.h"
#include "link.h"
#include "Device.h"
#include "deviceport.h"
#include "console.h"
#include "separateterminal.h"

// For LoadSettings function
#include "PC.h"
#include "router.h"

const int InsertTextButton = 10;

MainWindow::MainWindow():
    isLoaded(false),
    mSeparateTerminal(0)
{
    createActions();
    createToolBox();
    createMenus();
    createBottom();
    createLanguages();

    scene = new DiagramScene(menu_edit, this);
    scene->setSceneRect(QRectF(0, 0, 5000, 5000));
    connect(scene, SIGNAL(itemInserted(DiagramItem*)), this, SLOT(itemInserted(DiagramItem*)));
    connect(scene, SIGNAL(linkInserted(DiagramItem*,DiagramItem*,Link*)), this, SLOT(linkInserted(DiagramItem*,DiagramItem*,Link*)));
    connect(scene, SIGNAL(itemSelected(DiagramItem*)), this, SLOT(itemSelected(DiagramItem*)));
    connect(scene, SIGNAL(propertiesRequest(DiagramItem*)), this, SLOT(openSeparateTerminal(DiagramItem*)));
    createToolbars();

    QFrame *hr = new QFrame;
    hr->setFrameShape(QFrame::HLine);
    hr->setFrameShadow(QFrame::Sunken);

    QVBoxLayout *layout = new QVBoxLayout;
    QHBoxLayout *midLayout = new QHBoxLayout;
    midLayout->addWidget(toolBox);
    view = new QGraphicsView(scene);
    midLayout->addWidget(view);
    QHBoxLayout *bottomLayout = new QHBoxLayout;
    bottomLayout->addWidget(lblInfo);
    bottomLayout->addWidget(miniterminal);

    layout->addLayout(midLayout);
    layout->addWidget(hr);
    layout->addLayout(bottomLayout);

    QWidget *widget = new QWidget;
    widget->setLayout(layout);

    setCentralWidget(widget);
    setWindowTitle(tr("FreeCiscoSimulator"));
    setUnifiedTitleAndToolBarOnMac(true);
}
void MainWindow::buttonGroupClicked(int id)
{
    QList<QAbstractButton *> buttons = buttonGroup->buttons();
    foreach (QAbstractButton *button, buttons)
        if (buttonGroup->button(id) != button)
            button->setChecked(false);
        else
            if (!button->isChecked())
                scene->setMode(DiagramScene::MoveItem);
            else
            {
                scene->setItemType(DiagramItem::DiagramType(id));
                scene->setMode(DiagramScene::InsertItem);
            }
}

void MainWindow::deleteItem()
{
    foreach (QGraphicsItem *item, scene->selectedItems())
    {
        if (item->type() == Link::Type)
        {
            scene->removeItem(item);
            Link *link = qgraphicsitem_cast<Link *>(item);

            QList <Device::deviceLink*> _links;
            _links = link->startItem()->device->DeleteLink(link->endItem()->device);
            if (_links.count() > 1)
            {
                QString msg = QString("There is ").append("%1").arg(_links.count()).append(
                            " connection between this devices.").append("\nChoose what you want to delete:");
                int x = 0;
                foreach (Device::deviceLink *_link, _links)
                    msg.append(QString("\n%1").arg(++x).append(") ").append(_link->firstDevice->GetDeviceID()).append(
                                ":%1").arg(_link->firstPort->number).append(
                                ":").append(_link->secondDevice->GetDeviceID()).append(
                                ":%1").arg(_link->secondPort->number));
                bool ok;
                x = QInputDialog::getInt(this, "Cautition!", msg, 1, 1, x, 1, &ok);
                if (ok)
                {
                    _links.at(x-1)->firstDevice->DeleteLink(_links.at(x-1), true);
                }
                else
                {
                   return;
               }
            }

            link->startItem()->removeLink(link);
            link->endItem()->removeLink(link);
            delete item;
        }
    }

    foreach (QGraphicsItem *item, scene->selectedItems()) {
         if (item->type() == DiagramItem::Type) {
             qgraphicsitem_cast<DiagramItem *>(item)->removeLinks();
             qgraphicsitem_cast<DiagramItem *>(item)->device->DeleteLinks();
             qgraphicsitem_cast<DiagramItem *>(item)->removeDevice();
         }
         scene->removeItem(item);
         delete item;
     }
}

void MainWindow::pointerGroupClicked(int)
{
    scene->setMode(DiagramScene::Mode(pointerTypeGroup->checkedId()));
}

void MainWindow::bringToFront()
{
    if (scene->selectedItems().isEmpty())
        return;

    QGraphicsItem *selectedItem = scene->selectedItems().first();
    QList<QGraphicsItem *> overlapItems = selectedItem->collidingItems();

    qreal zValue = 0;
    foreach (QGraphicsItem *item, overlapItems) {
        if (item->zValue() >= zValue &&
            item->type() == DiagramItem::Type)
            zValue = item->zValue() + 0.1;
    }
    selectedItem->setZValue(zValue);
}

void MainWindow::sendToBack()
{
    if (scene->selectedItems().isEmpty())
        return;

    QGraphicsItem *selectedItem = scene->selectedItems().first();
    QList<QGraphicsItem *> overlapItems = selectedItem->collidingItems();

    qreal zValue = 0;
    foreach (QGraphicsItem *item, overlapItems) {
        if (item->zValue() <= zValue &&
            item->type() == DiagramItem::Type)
            zValue = item->zValue() - 0.1;
    }
    selectedItem->setZValue(zValue);
}

void MainWindow::itemInserted(DiagramItem *item)
{
    pointerTypeGroup->button(int(DiagramScene::MoveItem))->setChecked(true);
    scene->setMode(DiagramScene::Mode(pointerTypeGroup->checkedId()));
    buttonGroup->button(int(item->diagramType()))->setChecked(false);
    // TODO: realize property window
//    connect(item, SIGNAL(PropertiesRequest(DiagramItem*)), this, SLOT(ShowProperties(DiagramItem*)));
}

void MainWindow::linkInserted(DiagramItem *first, DiagramItem *second, Link *GLink)
{
    pointerTypeGroup->button(int(DiagramScene::MoveItem))->setChecked(true);
    scene->setMode(DiagramScene::Mode(pointerTypeGroup->checkedId()));

    Device::deviceLink *link = new Device::deviceLink;
    foreach(devicePort *port, first->device->Ports())
        if (!port->busy)
        {
            link->firstDevice = first->device;
            link->firstPort = port;
            link->firstPort->busy = true;
            break;
        }
    foreach(devicePort* port, second->device->Ports())
        if (!port->busy)
        {
            link->secondDevice = second->device;
            link->secondPort = port;
            link->secondPort->busy = true;
            break;
        }
    first->device->AddLink(link);
    second->device->AddLink(link);
    GLink->AssignLink(link);
    connect(link->firstPort, SIGNAL(SendPacket(packet&)),  link->secondPort, SLOT(RecivePacket(packet&)));
    connect(link->secondPort, SIGNAL(SendPacket(packet&)), link->firstPort, SLOT(RecivePacket(packet&)));
}

void MainWindow::sceneScaleChanged(const QString &scale)
{
    double newScale = scale.left(scale.indexOf(tr("%"))).toDouble() / 100.0;
    QMatrix oldMatrix = view->matrix();
    view->resetMatrix();
    view->translate(oldMatrix.dx(), oldMatrix.dy());
    view->scale(newScale, newScale);
}

void MainWindow::about()
{
    QMessageBox::about(this, tr("About FreeCiscoSimulator"),
                       tr("The <b>FreeCiscoSimulator</b> is a free simulator of Cisco devices"));
}

void MainWindow::changeLanguage()
{
    QMessageBox::warning(this, tr("Language change"), tr("You must restart application for the changes to take effect"));

    QSettings settings;

    if (settings.value("language").isValid())
        settings.remove("language");
    else
        settings.setValue("language", "ru");
}

void MainWindow::createToolBox()
{
    buttonGroup = new QButtonGroup(this);
    buttonGroup->setExclusive(false);
    connect(buttonGroup, SIGNAL(buttonClicked(int)), this, SLOT(buttonGroupClicked(int)));
    QGridLayout *layout = new QGridLayout;

    extern QString lastDeviceID;
    extern QString lastMACAddressOctet;
    QString buf = lastDeviceID;
    QString buf2 = lastMACAddressOctet;
    layout->addWidget(createCellWidget(tr("PC"), DiagramItem::PC_item), 0, 0);
    layout->addWidget(createCellWidget(tr("router"), DiagramItem::router_item),0, 1);
    // Uncomment this line, when commutator will be realized.
//    layout->addWidget(createCellWidget(tr("commutator"), DiagramItem::commutator_item), 1, 0);
    lastDeviceID = buf;
    lastMACAddressOctet = buf2;

    layout->setRowStretch(3, 10);
    layout->setColumnStretch(2, 10);

    QWidget *itemWidget = new QWidget;
    itemWidget->setLayout(layout);

    toolBox = new QToolBox;
    toolBox->setSizePolicy(QSizePolicy(QSizePolicy::Maximum, QSizePolicy::Ignored));
    toolBox->setMinimumWidth(itemWidget->sizeHint().width());
    toolBox->addItem(itemWidget, tr("Devices"));
}

void MainWindow::createActions()
{
    toFrontAction = new QAction(QIcon(":/images/bringtofront.png"),
                                tr("Bring to &Front"), this);
    toFrontAction->setShortcut(QString::fromUtf8("Ctrl+F"));
    toFrontAction->setStatusTip(tr("Bring item to front"));
    connect(toFrontAction, SIGNAL(triggered()),
            this, SLOT(bringToFront()));

    sendBackAction = new QAction(QIcon(":/images/sendtoback.png"), tr("Send to &Back"), this);
    sendBackAction->setShortcut(QString::fromUtf8("Ctrl+B"));
    sendBackAction->setStatusTip(tr("Send item to back"));
    connect(sendBackAction, SIGNAL(triggered()), this, SLOT(sendToBack()));

    deleteAction = new QAction(QIcon(":/images/delete.png"), tr("&Delete"), this);
    deleteAction->setShortcut(tr("Delete"));
    deleteAction->setStatusTip(tr("Delete item from diagram"));
    connect(deleteAction, SIGNAL(triggered()), this, SLOT(deleteItem()));

    file_create = new QAction(tr("&New"), this);
    file_create->setShortcut(QString::fromUtf8("CTRL+N"));
    connect(file_create, SIGNAL(triggered()), this, SLOT(NewProject()));

    file_open = new QAction(tr("&Open"), this);
    file_open->setShortcut(QString::fromUtf8("CTRL+O"));
    connect(file_open, SIGNAL(triggered()), this, SLOT(LoadProject()));

    file_save = new QAction(tr("&Save"), this);
    file_save->setShortcut(QString::fromUtf8("CTRL+S"));
    connect(file_save, SIGNAL(triggered()), this, SLOT(SaveProject()));

    file_save_as = new QAction(tr("Save as"), this);
    file_save_as->setShortcut(QString::fromUtf8("CTRL+SHIFT+S"));
    connect(file_save_as, SIGNAL(triggered()), this, SLOT(SaveProjectAs()));

    file_quit = new QAction(tr("&Quit"), this);
    file_quit->setShortcut(QString::fromUtf8("CTRL+Q"));
    file_quit->setStatusTip(tr("Quit FreeCiscoSimulator"));
    connect(file_quit, SIGNAL(triggered()), qApp, SLOT(quit()));

    edit_properties = new QAction(tr("&Properties"), this);
    edit_properties->setShortcut(QString::fromUtf8("CTRL+P"));
    connect(edit_properties, SIGNAL(triggered()), this, SLOT(ShowProperties()));

    aboutAction = new QAction(tr("A&bout"), this);
    aboutAction->setShortcut(QString::fromUtf8("Ctrl+B"));
    connect(aboutAction, SIGNAL(triggered()), this, SLOT(about()));
}

void MainWindow::createMenus()
{
    menu_file = menuBar()->addMenu(tr("File"));
    menu_file->addAction(file_create);
    menu_file->addAction(file_open);
    menu_file->addAction(file_save);
    menu_file->addAction(file_save_as);
    menu_file->addSeparator();
    menu_file->addAction(file_quit);

    menu_edit = menuBar()->addMenu(tr("Edit"));
    menu_edit->addAction(deleteAction);
    menu_edit->addSeparator();
    menu_edit->addAction(toFrontAction);
    menu_edit->addAction(sendBackAction);
    // TODO: realize property window
//    menu_edit->addSeparator();
//    menu_edit->addAction(edit_properties);

    menu_language = menuBar()->addMenu(tr("Options"))->addMenu(tr("Language"));

    menu_about = menuBar()->addMenu(tr("Help"));
    menu_about->addAction(aboutAction);
}

void MainWindow::createToolbars()
{
    editToolBar = addToolBar(tr("Edit"));
    editToolBar->addAction(deleteAction);
    editToolBar->addAction(toFrontAction);
    editToolBar->addAction(sendBackAction);

    QToolButton *pointerButton = new QToolButton;
    pointerButton->setCheckable(true);
    pointerButton->setChecked(true);
    pointerButton->setIcon(QIcon(":/images/pointer.png"));
    QToolButton *linePointerButton = new QToolButton;
    linePointerButton->setCheckable(true);
    linePointerButton->setIcon(QIcon(":/images/linepointer.png"));

    pointerTypeGroup = new QButtonGroup(this);
    pointerTypeGroup->addButton(pointerButton, int(DiagramScene::MoveItem));
    pointerTypeGroup->addButton(linePointerButton, int(DiagramScene::InsertLine));
    connect(pointerTypeGroup, SIGNAL(buttonClicked(int)), this, SLOT(pointerGroupClicked(int)));

    sceneScaleCombo = new QComboBox;
    QStringList scales;
    scales << "50%" << "75%" << "100%" << "125%" << "150%";
    sceneScaleCombo->addItems(scales);
    sceneScaleCombo->setCurrentIndex(2);
    connect(sceneScaleCombo, SIGNAL(currentIndexChanged(QString)), this, SLOT(sceneScaleChanged(QString)));

    pointerToolbar = addToolBar(tr("Pointer type"));
    pointerToolbar->addWidget(pointerButton);
    pointerToolbar->addWidget(linePointerButton);
    pointerToolbar->addWidget(sceneScaleCombo);
}

QWidget *MainWindow::createCellWidget(const QString &text,
                      DiagramItem::DiagramType type)
{

    DiagramItem item(type, menu_edit);
    QIcon icon(item.pixmap());

    QToolButton *button = new QToolButton;
    button->setIcon(icon);
    button->setIconSize(QSize(50, 50));
    button->setCheckable(true);
    buttonGroup->addButton(button, int(type));

    QGridLayout *layout = new QGridLayout;
    layout->addWidget(button, 0, 0, Qt::AlignHCenter);
    layout->addWidget(new QLabel(text), 1, 0, Qt::AlignCenter);

    QWidget *widget = new QWidget;
    widget->setLayout(layout);

    return widget;
}

void MainWindow::createBottom()
{
    lblInfo = new QLabel();
    lblInfo->setMinimumSize(200, 100);
    lblInfo->setMaximumWidth(250);
    lblInfo->setAlignment(Qt::AlignTop);
    lblInfo->setText(tr("Information"));

    miniterminal = new Console();
    miniterminal->setMinimumSize(300,120);
    miniterminal->setMaximumHeight(200);
}

void MainWindow::createLanguages()
{
    QActionGroup *group = new QActionGroup(this);

    QAction *langEnglish = new QAction(QString("English"), this);
    connect(langEnglish, SIGNAL(triggered()), SLOT(changeLanguage()));
    langEnglish->setCheckable(true);
    group->addAction(langEnglish);
    menu_language->addAction(langEnglish);

    if (QFileInfo("./translation_ru.qm").exists()) {
        QAction *langRussian = new QAction(QString::fromUtf8("Русский"), this);
        connect(langRussian, SIGNAL(triggered()), SLOT(changeLanguage()));
        langRussian->setCheckable(true);
        group->addAction(langRussian);
        menu_language->addAction(langRussian);

        QSettings settings;

        QVariant lang = settings.value("language");

        if (lang.isValid() && (lang.toString() == "ru")) {
            langRussian->setChecked(true);
             return;
        }
    }

    langEnglish->setChecked(true);
}

void MainWindow::toggleStatusbar()
{
    if (statusbar->isChecked())
        statusBar()->show();
    else
        statusBar()->hide();
}

/*void MainWindow::toggleTerminal()
{
    if (_terminal->isChecked())
        if (this->terminal == NULL)
        {
            this->terminal = new Terminal();
            emit TerminalCreated(this->terminal);
            this->terminal->show();
        }
        else
            this->terminal->show();
    else
        this->terminal->hide();
}*/

void MainWindow::itemSelected(DiagramItem *item)
{
    Device::deviceInfo info = item->device->GetInfo();

    QString text;
    text = QString(tr("Information")).append("\n");

    text.append(tr("ID")).append(":").append(info.id).append("\n");

    text.append(tr("Name")).append(":").append(info.name).append("\n");

    text.append(tr("IP")).append(": ");
    bool firstentry = true;
    foreach (QString ip, info.ipAddresses)
    {
        if (firstentry)
        {
            text.append(ip);
            firstentry = false;
        }
        else
            text.append("\n     ").append(ip);
    }
    text.append("\n");

    text.append(tr("Links")).append(": ");
    firstentry = true;
    foreach (QString link, info.links)
    {
            if (firstentry)
            {
                text.append(link);
                firstentry = false;
            }
            else
                text.append(", ").append(link);
    }

    this->lblInfo->setText(text);
}

Device* MainWindow::GetDeviceById(const QString &id)
{
    bool ok1, ok2;
    if (scene->items().isEmpty())
        return NULL;
    foreach (QGraphicsItem *item, scene->items()) {
         if (item->type() == DiagramItem::Type)
             if (qgraphicsitem_cast<DiagramItem *>(item)->device->GetInfo().id.toUInt(&ok1,16) == id.toUInt(&ok2,16))
                return qgraphicsitem_cast<DiagramItem *>(item)->device;
    }
    return NULL;
}

void MainWindow::ShowProperties()
{
    if (scene->selectedItems().count() != 1)
        return;
    foreach (QGraphicsItem *item, scene->selectedItems()) {
         if (item->type() == DiagramItem::Type) {
            QMessageBox::about(this, tr("Properties"), tr("<u>Selected </u>").append(
                                   qgraphicsitem_cast<DiagramItem *>(item)->device->GetDeviceID()));
         }
    }
}

void MainWindow::ShowProperties(DiagramItem *item)
{
    QMessageBox::about(this, tr("Properties"),
                       tr("<u>Selected </u>").append(item->device->GetDeviceID()));
}

void MainWindow::NewProject()
{
    if (scene->items().size() != 0)
    {
        QMessageBox::StandardButton btn = QMessageBox::question(this, tr("Warning!"),
                                                                tr("Current scene not empty!\nDo you want to save it?"),
                                                                QMessageBox::Save | QMessageBox::No | QMessageBox::Cancel);
        switch(btn)
        {
        case QMessageBox::Save:
            SaveProjectCommon();
            ClearProject();
            break;
        case QMessageBox::No:
            ClearProject();
            break;
        default:
            return;
            break;
        }
    }
}

void MainWindow::ClearProject()
{
    foreach(QGraphicsItem* item, scene->items())
    {
        if (item->type() == DiagramItem::Type)
        {
            DiagramItem* itm = qgraphicsitem_cast<DiagramItem *>(item);
            itm->setSelected(true);
            deleteItem();
        }
    }
    scene->clear();
    extern QString lastDeviceID;
    extern QString lastMACAddressOctet;
    lastDeviceID = "0x0";
    lastMACAddressOctet = "0x0";
}

void MainWindow::SaveProject()
{
    SaveProjectCommon();
}

void MainWindow::SaveProjectAs()
{
    SaveProjectCommon(true);
}

void MainWindow::SaveProjectCommon(const bool &isAs)
{
    QString filename;
    if (isLoaded && !isAs)
    {
        filename = ProjectFile;
    }
    else
    {
        filename = QFileDialog::getSaveFileName(this, tr("Save sheme"), getenv("HOME"), tr("Config files (*.ini)"));
    }
    // Check write permissions and clear file if reachable
    if (filename.isEmpty())
        return;
    QFile f(filename);
    QFileInfo fi(filename);
    if (!fi.exists() || !fi.isWritable() || !QFileInfo(fi.absoluteDir().absolutePath()).isWritable())
    {
        QMessageBox::critical(this, tr("Error save file"), tr("You don't have permissions to write this file."));
        return;
    }
    f.open(QIODevice::WriteOnly|QIODevice::Truncate);
    f.write("");
    f.resize(f.pos());
    f.close();

    SaveSheme(filename);
    isLoaded = true;
    ProjectFile = filename;
}

void MainWindow::SaveSheme(const QString& filename)
{
    QSettings mySettings(filename, QSettings::IniFormat);

    extern QString lastDeviceID;
    extern QString lastMACAddressOctet;

    mySettings.beginGroup("Global");
    mySettings.setValue("lastDeviceID", lastDeviceID);
    mySettings.setValue("lastMACAddressOctet", lastMACAddressOctet);
    mySettings.endGroup();

    quint8 link_number = 1;
    foreach (QGraphicsItem *item, scene->items()) {
        if (item->type() == DiagramItem::Type)
        {
             Device* dev = qgraphicsitem_cast<DiagramItem *>(item)->device;
             DiagramItem* itm = qgraphicsitem_cast<DiagramItem *>(item);

             mySettings.beginGroup(dev->GetDeviceID());

             switch(dev->GetType())
             {
             case Device::dev_PC:
                 mySettings.setValue("Type", "PC");
                 break;
             case Device::dev_router:
                 mySettings.setValue("Type", "router");
                 break;
             case Device::dev_switch:
                 mySettings.setValue("Type", "switch");
                 break;
             }

             mySettings.setValue("Name", dev->GetName());
             mySettings.setValue("PortsCount", QString("%1").arg(dev->Ports().count()));

             // Save Interfaces
             mySettings.setValue("InterfacesCount", QString("%1").arg(dev->Interfaces().count()));
             foreach (QString key, dev->Interfaces().keys())
             {
                 mySettings.setValue(QString("Interfaces/").append(dev->Interfaces()[key].name.replace("/","-")).append("/address"),
                               dev->Interfaces()[key].address.ip().toString().append(QString("/%1").arg(dev->Interfaces()[key].address.prefixLength())));
                 mySettings.setValue(QString("Interfaces/").append(dev->Interfaces()[key].name.replace("/","-")).append("/mac"),
                               dev->Interfaces()[key].hwaddr);
                 mySettings.setValue(QString("Interfaces/").append(dev->Interfaces()[key].name.replace("/","-")).append("/port"),
                               dev->Interfaces()[key].AssignedPort->number);
             }

             // Save routes table without autogenerated routes (such as local - loop interface for itself)
             quint8 i = 0;
             mySettings.beginWriteArray("routes");
             foreach (Device::route r, dev->routes())
             {
                 if (r.scope != "link")
                 {
                     mySettings.setArrayIndex(i);
                     mySettings.setValue(QString("net"), r.net.toString());
                     mySettings.setValue(QString("prefix"), QString("%1").arg(r.prefix));
                     mySettings.setValue(QString("via"), r.gw.toString());
                     mySettings.setValue(QString("dev"), r.dev);
//                     ***** Calculate automatic *****
//                     save.setValue(QString("routes[%1]/src").arg(i), r.src);
//                     save.setValue(QString("routes[%1]/proto").arg(i), r.proto);
//                     save.setValue(QString("routes[%1]/scope").arg(i), r.scope);
//                     save.setValue(QString("routes[%1]/metric").arg(i), r.metric);
                     ++i;
                 }
             }
             mySettings.endArray();

             mySettings.setValue("Position/x", itm->x());
             mySettings.setValue("Position/y", itm->y());

             dev = NULL;
             itm = NULL;
             mySettings.endGroup();
         }
         if (item->type() == Link::Type)
         {
             Link* link = qgraphicsitem_cast<Link *>(item);

             mySettings.beginGroup("Connections");

             mySettings.setValue(QString("Link/%1/from").arg(link_number),link->link()->firstDevice->GetDeviceID());
             mySettings.setValue(QString("Link/%1/to").arg(link_number),link->link()->secondDevice->GetDeviceID());
             mySettings.setValue(QString("Link/%1/sport").arg(link_number),QString("%1").arg(link->link()->firstPort->number));
             mySettings.setValue(QString("Link/%1/dport").arg(link_number),QString("%1").arg(link->link()->secondPort->number));
             ++link_number;

             mySettings.endGroup();
         }
     }
    mySettings.beginGroup("Connections");
    mySettings.setValue("Link/size", link_number-1);
    mySettings.endGroup();

    mySettings.sync();
}

void MainWindow::LoadProject()
{
    QString filename;
    filename = QFileDialog::getOpenFileName(this, tr("Save sheme"), getenv("HOME"), tr("Config files (*.ini)"));

    // Check read permissions
    if (filename.isEmpty())
        return;
    QFile f(filename);
    if (!f.exists())
    {
        QMessageBox::critical(this, tr("Error open file"), tr("File doesn't exist."));
        return;
    }
    else if (f.isReadable())
    {
        QMessageBox::critical(this, tr("Error open file"), tr("You don't have permissions to read this file."));
        return;
    }

    if (scene->items().size() != 0)
    {
        QMessageBox::StandardButton btn = QMessageBox::question(this, tr("Warning!"),
                                                                tr("Current scene not empty!\nDo you want to save it?"),
                                                                QMessageBox::Save | QMessageBox::No | QMessageBox::Cancel);
        switch(btn)
        {
        case QMessageBox::Save:
            SaveProjectCommon();
            ClearProject();
            break;
        case QMessageBox::No:
            ClearProject();
            break;
        default:
            return;
            break;
        }
    }

    QSettings mySettings(filename, QSettings::IniFormat);

    extern QString lastDeviceID;
    extern QString lastMACAddressOctet;

    foreach(QString section, mySettings.childGroups())
    {
        if (section == "Global" || section == "Connections")
        {
            continue;
        }
        else
        {
            bool ok;
            // Check if section is DeviceID then proceed
            int buf = section.toInt(&ok, 16);
            --buf;

            QString dev_id;
            if (ok)
            {
                dev_id = QString::number(buf, 16);
            }
            else
            {
                continue;
            }
            mySettings.beginGroup(section);

            Device* dev;
            DiagramItem::DiagramType dev_type;

            // Load ports
            quint8 ports_count = mySettings.value("PortsCount").toUInt(&ok);
            if (!ok || ports_count == 0)
            {
                mySettings.endGroup();
                continue;
            }

            // Load common fields and create device
            QString dev_name = mySettings.value("Name", "").toString();
            QString dev_stringType = mySettings.value("Type", "").toString();
            if (dev_stringType == "PC")
            {
                dev = new PC(dev_name, ports_count);
                dev_type = DiagramItem::PC_item;
            }
            else if (dev_stringType == "router")
            {
                if (ports_count < 2)
                {
                    mySettings.endGroup();
                    continue;
                }
                dev = new router(dev_name, ports_count);
                dev_type = DiagramItem::router_item;
            }
//            this must be switch adding

            // Link phisycal and graphical device abstractions and move it
            DiagramItem* dev_item = new DiagramItem(dev_type, menu_edit);
            dev_item->AssignDevice(dev);
            dev_item->setX(mySettings.value("Position/x").toReal());
            dev_item->setY(mySettings.value("Position/y").toReal());
            connect(dev_item, SIGNAL(itemSelected(DiagramItem*)), scene, SIGNAL(itemSelected(DiagramItem*)));

            // Load interfaces
            dev->EraseInterfaces();
            quint8 dev_if_count = mySettings.value("InterfacesCount", "1").toInt();
            for(int i=0; i<dev_if_count; i++)
            {
                QString if_name = "";
                if (dev->GetType() == Device::dev_PC)
                {
                    if_name = QString("eth%1").arg(i);
                }
                else if (dev->GetType() == Device::dev_router)
                {
                    if_name = QString("FastEthernet0/%1").arg(i);
                }
                QString if_name_forload = if_name;
                if_name_forload.replace("/","-");
                QString if_addr = mySettings.value(QString("Interfaces/").append(if_name_forload).append("/address"),
                                                   "").toString().section("/",0,0);
                quint8 if_prefix = mySettings.value(QString("Interfaces/").append(if_name_forload).append("/address"),
                                                   "").toString().section("/",1).toUInt(&ok, 10);
                QString if_hwaddr = mySettings.value(QString("Interfaces/").append(if_name_forload).append("/mac")).toString();
                quint8 if_port = mySettings.value(QString("Interfaces/").append(if_name_forload).append("/port")).toString().toUInt(&ok, 10);
                dev->SetupInterface(if_name, if_addr, if_prefix, if_hwaddr, if_port);
            }

            // Load routes
            int size = mySettings.beginReadArray("routes");
            for (int i=0; i<size; i++)
            {
                mySettings.setArrayIndex(i);
                dev->addRoute(mySettings.value("dev", "").toString(), mySettings.value("net", "").toString(),
                              mySettings.value("prefix", "").toInt(), mySettings.value("via", "").toString());
            }
            mySettings.endArray();

            // Change DeviceID and update sheme
            dev->SetDeviceID(dev_id);

            // Draw graphical device
            scene->addItem(dev_item);

            dev = NULL;
            mySettings.endGroup();
        }
    }

    foreach(QString section, mySettings.childGroups())
    {
        if (section == "Global")
        {
            mySettings.beginGroup("Global");
//          ***** TODO: check max id of created devices!!! *****
//            **************************
//            **************************
            lastDeviceID = mySettings.value("lastDeviceID", "0").toString();
            lastMACAddressOctet = mySettings.value("lastMACAddressOctet", "0").toString();
            mySettings.endGroup();
        }
        else if (section == "Connections")
        {
            mySettings.beginGroup("Connections");

            int size = mySettings.beginReadArray("Link");
            for (int i=0; i<size; i++)
            {
                mySettings.setArrayIndex(i);

                Link* gLink;
                Device::deviceLink* pLink = new Device::deviceLink();

                bool ok;
                QString dev1 = mySettings.value("from").toString();
                DiagramItem* dev1_itm;
                bool dev1_exist = false;
                quint8 dev1_port = mySettings.value("sport").toString().toUInt(&ok, 10);
                if (!ok)
                {
                    continue;
                }
                bool dev1_port_exist = false;
                QString dev2 = mySettings.value("to").toString();
                DiagramItem* dev2_itm;
                bool dev2_exist = false;
                quint8 dev2_port = mySettings.value("dport").toString().toUInt(&ok, 10);
                if (!ok)
                {
                    continue;
                }
                bool dev2_port_exist = false;

                if (dev1 == dev2)
                {
                    continue;
                }

                foreach (QGraphicsItem *item, scene->items())
                {
                    if (item->type() == DiagramItem::Type)
                    {
                        Device* dev = qgraphicsitem_cast<DiagramItem *>(item)->device;
                        DiagramItem* itm = qgraphicsitem_cast<DiagramItem *>(item);
                        if (dev1 == dev->GetDeviceID())
                        {
                            dev1_exist = true;
                            dev1_itm = itm;
                            foreach (devicePort* port, dev->Ports())
                            {
                                if (dev1_port == port->number && !port->busy)
                                {
                                    dev1_port_exist = true;
                                    pLink->firstDevice = dev;
                                    pLink->firstPort = port;
                                    break;
                                }
                            }
                        }
                        if (dev2 == dev->GetDeviceID())
                        {
                            dev2_exist = true;
                            dev2_itm = itm;
                            foreach (devicePort* port, dev->Ports())
                            {
                                if (dev2_port == port->number && !port->busy)
                                {
                                    dev2_port_exist = true;
                                    pLink->secondDevice = dev;
                                    pLink->secondPort = port;
                                    break;
                                }
                            }
                        }
                    }
                }

                if (dev1_exist && dev1_port_exist && dev2_exist && dev2_port_exist)
                {
                    gLink = new Link(dev1_itm, dev2_itm);
                    pLink->firstPort->busy = true;
                    pLink->secondPort->busy = true;
                    pLink->firstDevice->AddLink(pLink);
                    pLink->secondDevice->AddLink(pLink);
                    gLink->AssignLink(pLink);
                    connect(pLink->firstPort, SIGNAL(SendPacket(ether_frame&)),  pLink->secondPort, SLOT(RecivePacket(ether_frame&)));
                    connect(pLink->secondPort, SIGNAL(SendPacket(ether_frame&)), pLink->firstPort, SLOT(RecivePacket(ether_frame&)));

                    scene->addItem(gLink);
                    gLink->updatePosition();
                    gLink->setZValue(-9999);
                }
            }

            mySettings.endArray();

            mySettings.endGroup();
        }
        else
        {
            continue;
        }
    }
    
    ProjectFile = filename;
    isLoaded = true;
}


void MainWindow::openSeparateTerminal(DiagramItem *item)
{
    if (!mSeparateTerminal) {
        mSeparateTerminal = new SeparateTerminal();
    }

    if (mSeparateTerminal->isHidden())
        mSeparateTerminal->show();

    mSeparateTerminal->setConsole(item->device->GetDeviceID());

    mSeparateTerminal->activateWindow();
    mSeparateTerminal->raise();
}
