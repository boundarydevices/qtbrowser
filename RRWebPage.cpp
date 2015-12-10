#include "RRWebPage.h"
#include <QDebug>
#include <QDate>

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
//do something!
qDebug() << message << lineNumber << sourceID;
}
