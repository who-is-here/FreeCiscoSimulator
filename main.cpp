#include <QtGui>

#include "mainwindow.h"
#include "Device.h"

int main(int argv, char *args[])
{
    Q_INIT_RESOURCE(FreeCiscoSimulator);

    extern MainWindow *mw;
    QApplication app(argv, args);

    app.setApplicationName ("FreeCiscoSimulator");
    app.setOrganizationName("FreeCiscoSimulator");

    QSettings settings;

    QVariant lang = settings.value("language");

    QTranslator qtTranslator;
    QTranslator simulatorTranslator;

    if (lang.isValid()) {

        qtTranslator.load("qt_" + lang.toString(),
                QLibraryInfo::location(QLibraryInfo::TranslationsPath));
        app.installTranslator(&qtTranslator);

        simulatorTranslator.load("./translation_" + lang.toString());

        app.installTranslator(&simulatorTranslator);
    }

    mw = new MainWindow();
    mw->setGeometry(100, 100, 800, 500);
    mw->show();

    return app.exec();
}
