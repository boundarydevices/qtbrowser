/****************************************************************************
**
** Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of Nokia Corporation and its Subsidiary(-ies) nor
**     the names of its contributors may be used to endorse or promote
**     products derived from this software without specific prior written
**     permission.
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
** $QT_END_LICENSE$
**
****************************************************************************/

#include <QtGui>
#include <QLinkedList>
#include "kbdInput.h"
#include "bcInput.h"
#include "accelInput.h"
#include "compassInput.h"
#include "gpsInput.h"
#include "magstripe.h"
#include "process.h"
#include "rfid.h"
#include "print.h"
#include <QWebView>
#include <QtNetwork/QNetworkSession>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkCookieJar>
#include <unistd.h>

class QWebFrame;
class QWebView;
QT_BEGIN_NAMESPACE
class QLineEdit;
QT_END_NAMESPACE

class MyNetworkCookieJar : public QNetworkCookieJar {
	Q_OBJECT
public:
	MyNetworkCookieJar ( QObject * parent = 0 )
		: QNetworkCookieJar(parent){
		printf("%s\n", __func__ );
	}
	virtual ~MyNetworkCookieJar () {
		printf("%s\n", __func__ );
	}
	Q_INVOKABLE bool clear();
	virtual QList<QNetworkCookie> cookiesForUrl ( const QUrl & url ) const {
		printf("%p, %s: %s\n", this, __func__, url.toString().toUtf8().data() );
		return QNetworkCookieJar::cookiesForUrl(url);
	}
        virtual bool setCookiesFromUrl ( const QList<QNetworkCookie> & cookieList, const QUrl & url ) {
		printf("%s: %s\n", __func__, url.toString().toUtf8().data() );
		QList<QNetworkCookie>::const_iterator it = cookieList.begin();
		while (it != cookieList.end()) {
			QNetworkCookie cookie = *it++ ;
			printf( "\t%s: %s\n", cookie.name().constData(), cookie.value().constData());
		}
                return QNetworkCookieJar::setCookiesFromUrl(cookieList,url);
	}
};

class popupWebView_t : public QWebView {
    Q_OBJECT
public:
	explicit popupWebView_t(QWidget* parent = 0);
    virtual ~popupWebView_t();
protected slots:
    void finishLoading(bool);
    void loadStarted();
    void javaScriptWindowObjectCleared();
    void frameCreated(QWebFrame *frame);

    void netsession_closed ();
    void netsession_error ( QNetworkSession::SessionError error );
    void netsession_newConfigurationActivated ();
    void netsession_opened ();
    void netsession_preferredConfigurationChanged ( const QNetworkConfiguration & config, bool isSeamless );
    void netsession_stateChanged ( QNetworkSession::State state );

    void authenticationRequired ( QNetworkReply * reply, QAuthenticator * authenticator );
    void finished ( QNetworkReply * reply );
    void networkAccessibleChanged ( QNetworkAccessManager::NetworkAccessibility accessible );
    void proxyAuthenticationRequired ( const QNetworkProxy & proxy, QAuthenticator * authenticator );
    void sslErrors ( QNetworkReply * reply, const QList<QSslError> & errors );
};

class mainWebView_t : public QWebView {
public:
	explicit mainWebView_t(QWidget* parent = 0): QWebView(parent), corners_(0), cookieJar(0){}
protected:
	virtual QWebView *createWindow(QWebPage::WebWindowType type);
	void mouseMoveEvent ( QMouseEvent * ev );
	void mouseReleaseEvent ( QMouseEvent * ev );
private:
	unsigned corners_ ;
	MyNetworkCookieJar *cookieJar ;
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(const QUrl& url);
    ~MainWindow(void);

protected slots:
    void finishLoading(bool);
    void loadStarted();
    void javaScriptWindowObjectCleared();
    void frameCreated(QWebFrame *frame);

    void netsession_closed ();
    void netsession_error ( QNetworkSession::SessionError error );
    void netsession_newConfigurationActivated ();
    void netsession_opened ();
    void netsession_preferredConfigurationChanged ( const QNetworkConfiguration & config, bool isSeamless );
    void netsession_stateChanged ( QNetworkSession::State state );

    void authenticationRequired ( QNetworkReply * reply, QAuthenticator * authenticator );
    void finished ( QNetworkReply * reply );
    void networkAccessibleChanged ( QNetworkAccessManager::NetworkAccessibility accessible );
    void proxyAuthenticationRequired ( const QNetworkProxy & proxy, QAuthenticator * authenticator );
    void sslErrors ( QNetworkReply * reply, const QList<QSslError> & errors );

    void readStdin(int fd);
protected:
    void closeEvent ( QCloseEvent * event );

private:
    QLinkedList<QString> jsFiles ;
    QSocketNotifier	 stdinReady ;
    mainWebView_t *view;
    kbdInput_t	kbd ;
    bcInput_t   bc ;
    accelInput_t accel ;
    compassInput_t compass ;
    gpsInput_t gps ;
    magstripe_t magstripe ;
    rfidReader_t rfid ;
    process_t	 process ;
    printer_t	 printer ;
    MyNetworkCookieJar *jar ;
};
