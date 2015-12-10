#include "rrwebpage.h"
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
}@

Here is where I assign an instance of RRWebPage in html5applicationviewer.cpp:

@NavigationControllerPrivate::NavigationControllerPrivate(QWidget *parent, QGraphicsWebView *webView)
: m_webPage(0)
, m_webWidget(0)
, m_graphicsWebView(webView)
, m_webNavigation(0)
{
Q_UNUSED(parent);
m_graphicsWebView->setAcceptTouchEvents(true);
m_webPage = new RRWebPage;
m_graphicsWebView->setPage(m_webPage);
m_webNavigation = new WebNavigation(m_graphicsWebView, m_webPage);
m_webNavigation->setParent(m_graphicsWebView);
}
