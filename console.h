#ifndef CONSOLE_H
#define CONSOLE_H

#include <QtGui>

//class QTextCursor;
class Device;

class Console : public QPlainTextEdit
{
    Q_OBJECT
public:
    explicit Console(QWidget* parent = 0);
    void scrollDown();
    void cmdConnect(const QString& args);
    Device *device() const {return dev;}
protected:
    void keyPressEvent(QKeyEvent* event);
    void mousePressEvent(QMouseEvent*);
    void mouseDoubleClickEvent(QMouseEvent*);
    void contextMenuEvent(QContextMenuEvent*);
private:
    QString prompt;
    bool isLocked;
    bool isConnected;
    Device* dev;
    QStringList* history;
    int historyPos;
    QString _currentCommand;
    bool _keep_currentCommand;

    void cmdClear(void);

    void onEnter();
    void insertPrompt(bool insertNewBlock = true);
    void historyAdd(QString cmd);
    void historyBack();
    void historyForward();

signals:
    void onCommand(const QString&);
    void onChange(const QString&);
    void CommandInterrupted(const QString& cmd);
    void disconnectCommand(Console* console);
//    void onBreak(const QString&);
//    void Finished(const QString&);
public slots:
    void KeepCommandString();
    void CommandFinished(const QString & = QString());
    void ConsoleWriteRequest(const QString& textToWrite);
    void ConsoleWriteRequestHTML(const QString &textToWrite);
    void ChangePrompt(const QString& newPrompt)
        { prompt = newPrompt; }
    void cmdExit();
    void DeviceDeleted();
};

#endif // CONSOLE_H
