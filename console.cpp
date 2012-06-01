#include "console.h"
#include "Device.h"
#include "mainwindow.h"

Console::Console(QWidget *parent) :QPlainTextEdit(parent)
{
    prompt = "> ";

    QPalette p = palette();
    p.setColor(QPalette::Base, Qt::black);
    p.setColor(QPalette::Text, Qt::green);
    setPalette(p);

    history = new QStringList;
    historyPos = 0;
    insertPrompt(false);
    isLocked = false;
    isConnected = false;
    _keep_currentCommand = false;

//    connect(this, SIGNAL(onBreak(QString)), this, SLOT(CommandFinished()));
//    connect(this, SIGNAL(Finished(QString)), this, SLOT(CommandFinished()));
}

void Console::keyPressEvent(QKeyEvent* event)
{
    if(event->modifiers() == Qt::CTRL && event->key() == *QString("C").toAscii())
    {
        emit CommandInterrupted(_currentCommand);
        ConsoleWriteRequest("^Ctrl+C\n");
        CommandFinished();
//        emit onBreak("^Ctrl+C\n");
    }
    if(isLocked)
        return;
    //if(event->key() >= 0x20 && event->key() <= 0x7e
    if(event->key() >= Qt::Key_Space && event->key() <= Qt::Key_AsciiTilde
            && (event->modifiers() == Qt::NoModifier || event->modifiers() == Qt::ShiftModifier))
    {
        QPlainTextEdit::keyPressEvent(event);
    }
    if((event->key() == Qt::Key_Left ||
        event->key() == Qt::Key_Backspace) &&
        event->modifiers() == Qt::NoModifier &&
        textCursor().positionInBlock() > prompt.length())
    {
        QPlainTextEdit::keyPressEvent(event);
    }
    if((event->key() == Qt::Key_Right ||
        event->key() == Qt::Key_Delete ||
        event->key() == Qt::Key_End) &&
        event->modifiers() == Qt::NoModifier &&
        textCursor().positionInBlock() >= prompt.length())
    {
        QPlainTextEdit::keyPressEvent(event);
    }
//    if(event->key() == Qt::Key_Home && event->modifiers() == Qt::NoModifier)
    if(event->key() == Qt::Key_Return && event->modifiers() == Qt::NoModifier)
    {
        QTextCursor cursor = textCursor();
        cursor.movePosition(QTextCursor::End);
        setTextCursor(cursor);
        onEnter();
    }
    if(event->key() == Qt::Key_Up && event->modifiers() == Qt::NoModifier)
    {
        historyBack();
    }
    if(event->key() == Qt::Key_Down && event->modifiers() == Qt::NoModifier)
    {
        historyForward();
    }
    if(event->key() == Qt::Key_Question)
    {
        if (isConnected)
        {
            if (dev->GetType() == Device::dev_router)
            {
                onEnter();
            }
        }
    }
}

void Console::mousePressEvent(QMouseEvent*)
{
    setFocus();
}

void Console::mouseDoubleClickEvent(QMouseEvent*){}

void Console::contextMenuEvent(QContextMenuEvent*){}

void Console::onEnter()
{
    if(textCursor().position() == prompt.length())
    {
        insertPrompt();
        return;
    }
    QString cmd = textCursor().block().text().mid(prompt.length()).simplified();
    isLocked = true;
    historyAdd(cmd);
    QTextCharFormat format;
    format.setForeground(Qt::green);
    textCursor().setBlockCharFormat(format);
    this->_currentCommand = cmd;

    // If command is connect, then find pointer to wanted object
/*    if (QRegExp("connect .*|connect", Qt::CaseInsensitive).exactMatch(cmd))
        cmdConnect(cmd.mid(8));
    else if (QRegExp("exit .*|exit", Qt::CaseInsensitive).exactMatch(cmd))
        cmdExit(cmd.mid(5));
    else if (QRegExp("clear", Qt::CaseInsensitive).exactMatch(cmd))
        cmdClear();*/
    if (cmd.startsWith("connect"))
        cmdConnect(cmd.section(" ",1));
//    else if (cmd.startsWith("exit"))
//        cmdExit(cmd.section(" ",1));
    else if (cmd.startsWith("clear"))
        cmdClear();
    else
    {
        if (isConnected)
            emit onCommand(cmd);
        else
        {
            ConsoleWriteRequest(cmd + ": command unknown");
            CommandFinished();
        }
    }
}

void Console::CommandFinished(const QString &)
{
    insertPrompt();
    scrollDown();
    if (_keep_currentCommand)
    {
        textCursor().insertText(_currentCommand.replace("?", ""));
        _keep_currentCommand = false;
    }
    this->_currentCommand = "";
    isLocked = false;
}

void Console::ConsoleWriteRequest(const QString& textToWrite)
{
    textCursor().insertText("\n" + textToWrite);
    scrollDown();
}

void Console::ConsoleWriteRequestHTML(const QString& textToWrite)
{
    textCursor().insertHtml("<br/>" + textToWrite);
//    QString buf = textToWrite;
//    textCursor().insertHtml("<br/>" + buf.replace(QString("\n"), QString("<br/>")));
//    textCursor().insertHtml("<br/><pre>" + textToWrite + "</pre>");
    scrollDown();
}

void Console::insertPrompt(bool insertNewBlock)
{
    if(insertNewBlock)
        textCursor().insertBlock();
    QTextCharFormat format;
    format.setForeground(Qt::green);
    textCursor().setBlockCharFormat(format);
    textCursor().insertText(prompt);
//    textCursor().insertHtml("<pre>" + prompt + "</pre>");
    scrollDown();
}

void Console::scrollDown()
{
    QScrollBar *vbar = verticalScrollBar();
    vbar->setValue(vbar->maximum());
}

void Console::historyAdd(QString cmd)
{
    history->append(cmd);
    historyPos = history->length();
}

void Console::historyBack()
{
    if(!historyPos)
return;
    QTextCursor cursor = textCursor();
    cursor.movePosition(QTextCursor::StartOfBlock);
    cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
    cursor.removeSelectedText();
    cursor.insertText(prompt + history->at(historyPos-1));
//    cursor.insertHtml("<pre>" + history->at(historyPos - 1) + "</pre>");
    setTextCursor(cursor);
    historyPos--;
}

void Console::historyForward()
{
    if(historyPos == history->length())
return;
    QTextCursor cursor = textCursor();
    cursor.movePosition(QTextCursor::StartOfBlock);
    cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
    cursor.removeSelectedText();
    if(historyPos == history->length() - 1)
    {
        cursor.insertText(prompt);
//        cursor.insertHtml("<pre>" + prompt + "</pre>");
    }
    else
    {
        cursor.insertText(prompt + history->at(historyPos + 1));
//        cursor.insertHtml("<pre>" + history->at(historyPos + 1) + "</pre>");
    }
    setTextCursor(cursor);
    historyPos++;
}

void Console::cmdConnect(const QString& args)
{
    QString ret;
    if (args == "")
    {
        ret = "UASGE\nconnect <DeviceID>";
    }
    if (this->isConnected)
        ret = "already connected";
    else
    {
        extern MainWindow *mw;
        this->dev = mw->GetDeviceById(args);
        if (this->dev == NULL)
            ret = "unknown device";
        else
        {
            connect(this, SIGNAL(onCommand(QString)), this->dev, SLOT(onExecuteCommand(QString)));
            connect(this->dev, SIGNAL(KeepCommandRequest()), this, SLOT(KeepCommandString()));
            connect(this->dev, SIGNAL(CommandReturn()), this, SLOT(CommandFinished()));
            connect(this->dev, SIGNAL(ConsoleWrite(QString)) , this, SLOT(ConsoleWriteRequest(QString)));
            connect(this->dev, SIGNAL(ConsoleWriteHTML(QString)), this, SLOT(ConsoleWriteRequestHTML(QString)));
            connect(this->dev, SIGNAL(ChangePrompt(QString)), this, SLOT(ChangePrompt(QString)));
            connect(this, SIGNAL(CommandInterrupted(QString)), this->dev, SLOT(onCommandInterrupt(QString)));
            connect(this->dev, SIGNAL(DisconnectConsole()), this, SLOT(cmdExit()));
            connect(this->dev, SIGNAL(destroyed()), this, SLOT(DeviceDeleted()));
            this->isConnected = true;
            ret = "connected to " + args;
        }
    }

    ConsoleWriteRequest(ret);
    CommandFinished();
}

void Console::cmdExit()
{
//    QString ret = args;
//    if (args.isEmpty())
//        ret = "0";
//
//    if (this->isConnected)
//    {
    disconnect(this, SIGNAL(onCommand(QString)), this->dev, SLOT(onExecuteCommand(QString)));
    disconnect(this->dev, SIGNAL(KeepCommandRequest()), this, SLOT(KeepCommandString()));
    disconnect(this->dev, SIGNAL(CommandReturn()), this, SLOT(CommandFinished()));
    disconnect(this->dev, SIGNAL(ConsoleWrite(QString)) , this, SLOT(ConsoleWriteRequest(QString)));
    disconnect(this->dev, SIGNAL(ConsoleWriteHTML(QString)), this, SLOT(ConsoleWriteRequestHTML(QString)));
    disconnect(this->dev, SIGNAL(ChangePrompt(QString)), this, SLOT(ChangePrompt(QString)));
    disconnect(this, SIGNAL(CommandInterrupted(QString)), this->dev, SLOT(onCommandInterrupt(QString)));
    disconnect(this->dev, SIGNAL(DisconnectConsole()), this, SLOT(cmdExit()));
    disconnect(this->dev, SIGNAL(destroyed()), this, SLOT(DeviceDeleted()));
    this->isConnected = false;
    ConsoleWriteRequest("connection closed with "+this->dev->GetDeviceID());
    CommandFinished();
    this->dev = NULL;
//    }
//    else
//    {
//        ConsoleWriteRequest("exiting...");
//        CommandFinished();
////        emit Finished("exiting...");
//        //this->destroy();
//    }
}

void Console::DeviceDeleted()
{
    if (this->isConnected)
    {
        this->isConnected = false;
        this->_currentCommand = " ";
        ConsoleWriteRequest("connection is broken...");
        CommandFinished();
        this->dev = NULL;
    }
}

void Console::cmdClear()
{
    clear();
    insertPrompt();
    isLocked = false;
}

void Console::KeepCommandString()
{
    _keep_currentCommand = true;
}
