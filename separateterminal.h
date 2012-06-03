#ifndef SEPARATETERMINAL_H
#define SEPARATETERMINAL_H

#include <QWidget>

class QTabWidget;
class Console;

class SeparateTerminal : public QWidget
{
    Q_OBJECT
public:
    explicit SeparateTerminal(QWidget *parent = 0);
    void setConsole(const QString &id);

private slots:
    void closeTab(int index);
    void exitHandler(Console* console);
private:
    QTabWidget *mTabWidget;
};

#endif // SEPARATETERMINAL_H
