#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "diagramitem.h"

class DiagramScene;

class QAction;
class QToolBox;
class QComboBox;
class QButtonGroup;
class QLabel;
class QLineEdit;
class QGraphicsTextItem;
class QAbstractButton;
class QGraphicsView;
class SeparateTerminal;

class Console;

class MainWindow : public QMainWindow
{
    Q_OBJECT

    public:
       MainWindow();
       Device* GetDeviceById(const QString& id);

    private slots:
        void buttonGroupClicked(int id);
        void deleteItem();
        void pointerGroupClicked(int);
        void bringToFront();
        void sendToBack();
        void itemInserted(DiagramItem* item);
        void linkInserted(DiagramItem* first, DiagramItem* second, Link* GLink);
        void itemSelected(DiagramItem* item);
        void openSeparateTerminal(DiagramItem* item);
        void ShowProperties();
        void ShowProperties(DiagramItem* item);
        void sceneScaleChanged(const QString& scale);
        void about();
        void changeLanguage();

        void SaveProject();
        void SaveProjectAs();
        void LoadProject();
        void NewProject();

        //void toggleTerminal();
        void toggleStatusbar();
    private:
        bool isLoaded;
        QString ProjectFile;
        void SaveSheme(const QString& filename);
        void SaveProjectCommon(const bool& isAs = false);
        void ClearProject();

        void createToolBox();
        void createActions();
        void createMenus();
        void createToolbars();
        void createLanguages();
        QWidget* createCellWidget(const QString& text, DiagramItem::DiagramType type);
        void createBottom();

        DiagramScene* scene;
        QGraphicsView* view;

        QAction* addAction;
        QAction* deleteAction;

        QAction* edit_properties;

        QAction* toFrontAction;
        QAction* sendBackAction;
        QAction* aboutAction;

        QMenu* menu_file;
        QMenu* menu_edit;
        QMenu* menu_language;
        QMenu* menu_about;

        QAction* file_create;
        QAction* file_open;
        QAction* file_save;
        QAction* file_save_as;
        QAction* file_quit;

        QToolBar* editToolBar;
        QToolBar* pointerToolbar;

        QComboBox* sceneScaleCombo;

        QLabel* lblInfo;
        Console* miniterminal;

        QToolBox* toolBox;
        QButtonGroup* buttonGroup;
        QButtonGroup* pointerTypeGroup;
        QAction* lineAction;

        QAction* statusbar;

        SeparateTerminal *mSeparateTerminal;
};

#endif
