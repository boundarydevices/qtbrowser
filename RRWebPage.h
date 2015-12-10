#ifndef RRWEBPAGE_H
#define RRWEBPAGE_H

#include <QWebPage>

class RRWebPage : public QWebPage
{
Q_OBJECT
public:
explicit RRWebPage(QObject *parent = 0);

void test();

protected:
void javaScriptConsoleMessage( const QString & message, int lineNumber, const QString & sourceID );

signals:

public slots:

};

#endif // RRWEBPAGE_H
