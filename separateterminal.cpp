#include "separateterminal.h"

#include <QHBoxLayout>
#include <QTabWidget>

#include "console.h"
#include "Device.h"

SeparateTerminal::SeparateTerminal(QWidget *parent) :
    QWidget(parent),
    mTabWidget(new QTabWidget())
{
    setAttribute(Qt::WA_QuitOnClose, false);

    mTabWidget->setTabsClosable(true);
    connect(mTabWidget, SIGNAL(tabCloseRequested(int)), SLOT(closeTab(int)));

    QHBoxLayout *layout = new QHBoxLayout();
    layout->setMargin(0);

    layout->addWidget(mTabWidget);

    setLayout(layout);
}


void SeparateTerminal::setConsole(const QString &id)
{
    const int tabCount = mTabWidget->count();

    for (int i=0; i<tabCount; ++i) {
        Console *console = qobject_cast<Console*>(mTabWidget->widget(i));

        if (console->device()->GetDeviceID() == id) {
            mTabWidget->setCurrentIndex(i);
            return;
        }
    }

    Console *console = new Console();

    console->setMinimumWidth (640);
    console->setMinimumHeight(360);

    connect(console, SIGNAL(disconnectCommand(Console*)), SLOT(exitHandler(Console*)));

    mTabWidget->setCurrentIndex(mTabWidget->addTab(console, tr("Device %1").arg(id)));

    console->cmdConnect(id);
}


void SeparateTerminal::closeTab(int index)
{
    QWidget *console = mTabWidget->widget(index);
    mTabWidget->removeTab(index);

    console->deleteLater();

    if (!mTabWidget->count())
        close();
}


void SeparateTerminal::exitHandler(Console *console)
{
    const int tabCount = mTabWidget->count();

    for (int i=0; i<tabCount; ++i) {
        Console *consoleTab = qobject_cast<Console*>(mTabWidget->widget(i));

        if (console==consoleTab) {
            closeTab(i);
            return;
        }
    }
}
