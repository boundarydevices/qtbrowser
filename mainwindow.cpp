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
#include <QtWebKit>
#include <QtNetwork/QNetworkConfigurationManager>
#include "mainwindow.h"
#include <QtNetwork/QNetworkCookieJar>
#include <fcntl.h>

class MyNetworkCookieJar : public QNetworkCookieJar {
public:
	MyNetworkCookieJar ( QObject * parent = 0 )
		: QNetworkCookieJar(parent){
		printf("%s\n", __func__ );
	}
	virtual ~MyNetworkCookieJar () {
		printf("%s\n", __func__ );
	}
	virtual QList<QNetworkCookie> cookiesForUrl ( const QUrl & url ) const {
		printf("%s: %s\n", __func__, url.toString().toUtf8().data() );
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

#ifndef ARRAYSIZE
#define ARRAYSIZE(__arr) (sizeof(__arr)/sizeof(__arr[0]))
#endif

static bool const forgivable_ssl_errors[] = {
    false,      // QSslError::NoError	0
    false,      // QSslError::UnableToGetIssuerCertificate	1
    false,      // QSslError::UnableToDecryptCertificateSignature	2
    false,      // QSslError::UnableToDecodeIssuerPublicKey	3
    false,      // QSslError::CertificateSignatureFailed	4
    true,       // QSslError::CertificateNotYetValid	5
    false,      // QSslError::CertificateExpired	6
    false,      // QSslError::InvalidNotBeforeField	7
    false,      // QSslError::InvalidNotAfterField	8
    false,      // QSslError::SelfSignedCertificate	9
    false,      // QSslError::SelfSignedCertificateInChain	10
    true,       // QSslError::UnableToGetLocalIssuerCertificate	11
    false,      // QSslError::UnableToVerifyFirstCertificate	12
    false,      // QSslError::CertificateRevoked	13
    false,      // QSslError::InvalidCaCertificate	14
    false,      // QSslError::PathLengthExceeded	15
    false,      // QSslError::InvalidPurpose	16
    true,       // QSslError::CertificateUntrusted	17
    false,      // QSslError::CertificateRejected	18
    false,      // QSslError::SubjectIssuerMismatch	19
    false,      // QSslError::AuthorityIssuerSerialNumberMismatch	20
    false,      // QSslError::NoPeerCertificate	21
    true,       // QSslError::HostNameMismatch	22
    false       // QSslError::NoSslSupport	23
};

static bool ssl_errors_forgivable(const QList<QSslError> & errors )
{
    bool forgivable = true ;
    printf("%s\n", __func__ );
    QList<QSslError>::const_iterator it = errors.begin();
    while (it != errors.end()) {
        QSslError err = *it++ ;
        unsigned errcode = (int)err.error();
        printf( "0x%x: %s\n", (int)err.error(), err.errorString().toUtf8().data());
        forgivable = forgivable 
                     && (errcode < ARRAYSIZE(forgivable_ssl_errors))
                     && forgivable_ssl_errors[errcode];
    }
    return forgivable ;
}

popupWebView_t::popupWebView_t(QWidget* parent)
    : QWebView(parent){
    printf( "popup constructor: parent == %p\n", parent);
}

popupWebView_t::~popupWebView_t() {
	printf("%s\n", __func__ );
}

void popupWebView_t::finishLoading(bool){ printf ("%s: %llu bytes\n", __func__, page()->totalBytes()); }
void popupWebView_t::loadStarted(){ printf ("%s\n", __func__); }
void popupWebView_t::javaScriptWindowObjectCleared(){ printf ("%s\n", __func__); }
void popupWebView_t::frameCreated(QWebFrame *frame){ printf ("%s\n", __func__); }

void popupWebView_t::netsession_closed (){ printf ("%s\n", __func__); }
void popupWebView_t::netsession_error ( QNetworkSession::SessionError error ){ printf ("%s\n", __func__); }
void popupWebView_t::netsession_newConfigurationActivated (){ printf ("%s\n", __func__); }
void popupWebView_t::netsession_opened (){ printf ("%s\n", __func__); }
void popupWebView_t::netsession_preferredConfigurationChanged ( const QNetworkConfiguration & config, bool isSeamless ){ printf ("%s\n", __func__); }
void popupWebView_t::netsession_stateChanged ( QNetworkSession::State state ){ printf ("%s\n", __func__); }

void popupWebView_t::authenticationRequired ( QNetworkReply * reply, QAuthenticator * authenticator ){ printf ("%s\n", __func__); }
void popupWebView_t::finished ( QNetworkReply * reply ){ printf("%s:%s\n", __func__, reply->url().toString().toUtf8().data() ); }
void popupWebView_t::networkAccessibleChanged ( QNetworkAccessManager::NetworkAccessibility accessible ){ printf ("%s\n", __func__); }
void popupWebView_t::proxyAuthenticationRequired ( const QNetworkProxy & proxy, QAuthenticator * authenticator ){ printf ("%s\n", __func__); }
void popupWebView_t::sslErrors ( QNetworkReply * reply, const QList<QSslError> & errors )
{ 
    bool forgivable = ssl_errors_forgivable(errors);
    if (forgivable) {
        reply->ignoreSslErrors(errors);
    }
    printf ("%s: %d\n", __func__, forgivable); 
}

QWebView *mainWebView_t::createWindow(QWebPage::WebWindowType type)
{
        QWebView *rval = new popupWebView_t(this);

	printf ("%s: %p, type %d\n", __func__, rval, type );
	if (rval) {
		rval->setPage(new QWebPage(rval));
		rval->setRenderHints( QPainter::Antialiasing 
				| QPainter::TextAntialiasing );
		rval->showNormal();
		connect(rval, SIGNAL(loadFinished(bool)), rval, SLOT(finishLoading(bool)));
		QNetworkAccessManager *na_manager = rval->page()->networkAccessManager();
		printf( "loaded network access manager: %p\n",na_manager);
		connect(na_manager, SIGNAL(authenticationRequired (QNetworkReply *, QAuthenticator *)), rval, SLOT(authenticationRequired (QNetworkReply *, QAuthenticator *)));
		connect(na_manager, SIGNAL(finished (QNetworkReply *)), rval, SLOT(finished (QNetworkReply *)));
		connect(na_manager, SIGNAL(networkAccessibleChanged (QNetworkAccessManager::NetworkAccessibility)), rval, SLOT(networkAccessibleChanged (QNetworkAccessManager::NetworkAccessibility)));
		connect(na_manager, SIGNAL(proxyAuthenticationRequired (const QNetworkProxy &, QAuthenticator *)), rval, SLOT(proxyAuthenticationRequired (const QNetworkProxy &, QAuthenticator *)));
		connect(na_manager, SIGNAL(sslErrors(QNetworkReply *, const QList<QSslError> &)), rval, SLOT(sslErrors(QNetworkReply *, const QList<QSslError> &)));
		printf("Network access manager signals connected\n");
		
		connect(rval->page()->mainFrame(), SIGNAL(loadStarted()), rval, SLOT(loadStarted()));
		connect(rval->page()->mainFrame(), SIGNAL(javaScriptWindowObjectCleared()), rval, SLOT(javaScriptWindowObjectCleared()));
		connect(rval->page(), SIGNAL(frameCreated(QWebFrame*)), rval, SLOT(frameCreated(QWebFrame*)));
                QNetworkCookieJar *old_jar = na_manager->cookieJar();
		na_manager->setCookieJar(new MyNetworkCookieJar(old_jar?old_jar->parent() : 0));
		printf( "%s: set cookie jar to %p\n", __func__, na_manager->cookieJar());
	}
	return rval ;
}

MainWindow::MainWindow(const QUrl& url)
	: QMainWindow(0,Qt::FramelessWindowHint)
	, stdinReady(fileno(stdin), stdinReady.Read)
	, kbd()
	, bc()
	, magstripe()
	, rfid()
{
    QNetworkProxyFactory::setUseSystemConfiguration(true);

    QNetworkConfigurationManager manager;
    printf("network capabilities: 0x%x\n", (int)manager.capabilities());
    // Get saved network configuration
    QSettings settings(QSettings::UserScope, QLatin1String("Trolltech"));
    settings.beginGroup(QLatin1String("QtNetwork"));
    const QString id = settings.value(QLatin1String("DefaultNetworkConfiguration")).toString();
    settings.endGroup();
    
    printf("have network config mgr\n");
    // If the saved network configuration is not currently discovered use the system default
    QNetworkConfiguration config = manager.configurationFromIdentifier(id);
    if ((config.state() & QNetworkConfiguration::Discovered) !=
        QNetworkConfiguration::Discovered) {
        config = manager.defaultConfiguration();
    }
    
    QNetworkSession *networkSession ;
    networkSession = new QNetworkSession(config, this);
    networkSession->open();
    printf("network session opened\n");
    connect(networkSession, SIGNAL(closed ()), this, SLOT(netsession_closed()));
    connect(networkSession, SIGNAL(error (QNetworkSession::SessionError)), this, SLOT(netsession_error(QNetworkSession::SessionError)));
    connect(networkSession, SIGNAL(newConfigurationActivated ()), this, SLOT(netsession_newConfigurationActivated()));
    connect(networkSession, SIGNAL(opened ()), this, SLOT(netsession_opened()));
    connect(networkSession, SIGNAL(preferredConfigurationChanged (const QNetworkConfiguration &, bool)), this, SLOT(netsession_preferredConfigurationChanged(const QNetworkConfiguration &, bool)));
    connect(networkSession, SIGNAL(stateChanged (QNetworkSession::State)), this, SLOT(netsession_stateChanged(QNetworkSession::State)));

    QRect screen_size = QApplication::desktop()->screenGeometry();

    char *jsPreload = getenv("JSPRELOAD");
    if (jsPreload) {
	    jsPreload = strdup(jsPreload);
	    char *next = strtok(jsPreload,":");
	    while (next) {
		    QFile file;
		    file.setFileName(next);
		    file.open(QIODevice::ReadOnly);
		    jsFiles.push_back(file.readAll());
		    file.close();
		    printf( "loaded %s\n", next);
		    next = strtok(0,":");
	    }
	    free(jsPreload);
    }
        
    view = new mainWebView_t(this);
    view->setRenderHints( QPainter::Antialiasing 
                        | QPainter::TextAntialiasing );

    view->load(url);
    connect(view, SIGNAL(loadFinished(bool)), SLOT(finishLoading(bool)));
    QNetworkAccessManager *na_manager = view->page()->networkAccessManager();
    printf( "loaded network access manager: %p\n",na_manager);
    connect(na_manager, SIGNAL(authenticationRequired (QNetworkReply *, QAuthenticator *)), this, SLOT(authenticationRequired (QNetworkReply *, QAuthenticator *)));
    connect(na_manager, SIGNAL(finished (QNetworkReply *)), this, SLOT(finished (QNetworkReply *)));
    connect(na_manager, SIGNAL(networkAccessibleChanged (QNetworkAccessManager::NetworkAccessibility)), this, SLOT(networkAccessibleChanged (QNetworkAccessManager::NetworkAccessibility)));
    connect(na_manager, SIGNAL(proxyAuthenticationRequired (const QNetworkProxy &, QAuthenticator *)), this, SLOT(proxyAuthenticationRequired (const QNetworkProxy &, QAuthenticator *)));
    connect(na_manager, SIGNAL(sslErrors(QNetworkReply *, const QList<QSslError> &)), this, SLOT(sslErrors(QNetworkReply *, const QList<QSslError> &)));
    printf("Network access manager signals connected\n");

    QNetworkCookieJar *old_jar = na_manager->cookieJar();
    na_manager->setCookieJar(new MyNetworkCookieJar(old_jar?old_jar->parent() : 0));
    printf( "set cookie jar to %p\n", na_manager->cookieJar());

    connect(view->page()->mainFrame(), SIGNAL(loadStarted()), this, SLOT(loadStarted()));
    connect(view->page()->mainFrame(), SIGNAL(javaScriptWindowObjectCleared()), this, SLOT(javaScriptWindowObjectCleared()));
    connect(view->page(), SIGNAL(frameCreated(QWebFrame*)), this, SLOT(frameCreated(QWebFrame*)));
    setCentralWidget(view);
    resize(QSize(screen_size.width(),screen_size.height()));
    view->page()->settings()->setAttribute(QWebSettings::JavascriptCanOpenWindows, true);

    fcntl(0, F_SETFL, fcntl(0, F_GETFL)|O_NONBLOCK);
    QObject::connect(&stdinReady, SIGNAL(activated(int)), this, SLOT(readStdin(int)));
}

void MainWindow::finishLoading(bool success)
{
	printf( "%s(%u): %llu bytes, %u scripts\n", __func__, success, view->page()->totalBytes(), jsFiles.size() );
	QLinkedListIterator<QString> it(jsFiles);
	while (it.hasNext()) {
		view->page()->mainFrame()->evaluateJavaScript(it.next());
	}
        view->page()->settings()->setAttribute(QWebSettings::JavascriptCanOpenWindows, true);
}

void MainWindow::loadStarted(void)
{
	printf( "%s\n", __func__ );
}

void MainWindow::javaScriptWindowObjectCleared(void)
{
	printf( "%s\n", __func__ );
        view->page()->mainFrame()->addToJavaScriptWindowObject("keyboard",&kbd);
        view->page()->mainFrame()->addToJavaScriptWindowObject("scanner",&bc);
        view->page()->mainFrame()->addToJavaScriptWindowObject("magstripe",&magstripe);
        view->page()->mainFrame()->addToJavaScriptWindowObject("rfid",&rfid);
        view->page()->mainFrame()->addToJavaScriptWindowObject("accel",&accel);
}

void MainWindow::frameCreated(QWebFrame *frame)
{
    printf( "frame created\n");
}

void MainWindow::netsession_closed ()
{
    printf("%s\n", __func__ );
}
void MainWindow::netsession_error ( QNetworkSession::SessionError error )
{
    printf("%s\n", __func__ );
}
void MainWindow::netsession_newConfigurationActivated ()
{
    printf("%s\n", __func__ );
}
void MainWindow::netsession_opened ()
{
    printf("%s\n", __func__ );
}
void MainWindow::netsession_preferredConfigurationChanged ( const QNetworkConfiguration & config, bool isSeamless )
{
    printf("%s\n", __func__ );
}
void MainWindow::netsession_stateChanged ( QNetworkSession::State state )
{
    printf("%s\n", __func__ );
}

void MainWindow::authenticationRequired ( QNetworkReply * reply, QAuthenticator * authenticator )
{
    printf("%s\n", __func__ );
}

void MainWindow::finished ( QNetworkReply * reply )
{
    printf("%s:%s\n", __func__, reply->url().toString().toUtf8().data() );
}

void MainWindow::networkAccessibleChanged ( QNetworkAccessManager::NetworkAccessibility accessible )
{
    printf("%s\n", __func__ );
}

void MainWindow::proxyAuthenticationRequired ( const QNetworkProxy & proxy, QAuthenticator * authenticator )
{
    printf("%s\n", __func__ );
}

void MainWindow::sslErrors ( QNetworkReply * reply, const QList<QSslError> & errors )
{
    bool forgivable = ssl_errors_forgivable(errors);
    if (forgivable) {
        reply->ignoreSslErrors(errors);
    }
}

#include <QDebug>

void MainWindow::readStdin(int fd)
{
	char inbuf[2048];
	int numRead ;
	while (0 < (numRead = read(fd,inbuf,sizeof(inbuf)-1))) {
		inbuf[numRead] = 0 ;
		QString qs(inbuf);
                QVariant result = view->page()->mainFrame()->evaluateJavaScript(qs);
		qDebug() << result ;
	}
}

