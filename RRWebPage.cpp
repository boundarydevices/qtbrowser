#include "RRWebPage.h"
#include <QDebug>
#include <QDate>
#include <stdio.h>

RRWebPage::RRWebPage(QObject *parent) :
QWebPage(parent)
{
}

void RRWebPage::test()
{
//say hello!
qDebug() << "Date:" << QDate::currentDate();
}

void RRWebPage::javaScriptConsoleMessage( const QString & message, int lineNumber, const QString & sourceID )
{
printf("CONSOLE.LOG(%d): %s. SourceID: %s\n",lineNumber,message,sourceID);
//qDebug() << message << lineNumber << sourceID;
}
