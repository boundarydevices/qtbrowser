/* rfid.cpp
 * 
 * Defines the Qt bindings for the stronglink_t 
 * proximity smart card reader class as described
 * in rfid.h.
 *
 */
#include "rfid.h"
#include <unistd.h>
#include <stdio.h>

rfidReader_t::rfidReader_t()
	: notifier(0)
	, sldev(0)
	, waiting(false)
	, ispresent(false)
	, timer(this)
{
	char const *devname = getenv("RFIDDEV");
	if (devname) {
		sldev = new stronglink_t(devname);
		if (sldev->isOpen()) {
                        notifier = new QSocketNotifier(sldev->getFd(),notifier->Read);
			QObject::connect(notifier, SIGNAL(activated(int)), this, SLOT(readData(int)));
			connect(&timer, SIGNAL(timeout()), this, SLOT(timeout()));
			timer.setInterval(100);
			timer.start();
		} else {
			delete sldev ; sldev = 0 ;
		}
	}
}

rfidReader_t::~rfidReader_t()
{
	timer.stop();
	if (notifier) {
		delete notifier ;
		notifier = 0 ;
	}
	if (sldev) {
		delete sldev ;
		sldev = 0 ;
	}
}

bool rfidReader_t::isPresent(void) const {
	long long serial ;
	return sldev && sldev->serialNumber(serial);
}

static QString const cardTypeNames[] = {
   QString("None")
,  QString("Mifare Standard 1K card")
,  QString("Mifare Pro card")
,  QString("Mifare UltraLight card")
,  QString("Mifare Standard 4K card")
,  QString("Mifare ProX card")
,  QString("Mifare DesFire card")
};

static unsigned const numCardTypeNames = sizeof(cardTypeNames)/sizeof(cardTypeNames[0]);

void rfidReader_t::readData(int fd)
{
	if (notifier && sldev) {
		enum stronglink_t::response_e resp ;
		void const *rxdata ;
		unsigned rxlen ;
		void *context ;
		if (sldev->receive(resp,rxdata,rxlen,context)) {
			waiting = false ;
			long long serial ;
			bool nowPresent = sldev->serialNumber(serial);
			if (nowPresent != ispresent) {
				if (nowPresent) {
					char buf[32];
					snprintf(buf,sizeof(buf)-1,"%llu",serial);
					QString cardtype(cardTypeNames[0]);
					if (rxdata&&rxlen) {
						char const *bytes = (char const *)rxdata ;
						unsigned type = bytes[rxlen-1];
						if (type < numCardTypeNames)
							cardtype = cardTypeNames[type];
					}
					emit cardPresent(QString(buf),cardtype);
				} else {
					emit cardRemoved();
				}
				ispresent = nowPresent ;
			}
		}
	}
}

bool rfidReader_t::send(int cmd, QString data)
{
	if (data.isEmpty()) {
                return send((stronglink_t::command_e)cmd,0,0,0);
	} else {
		QByteArray bytes(data.toAscii());
		return send((stronglink_t::command_e)cmd,bytes.constData(),bytes.length(),0);
	}
}

bool rfidReader_t::send(stronglink_t::command_e cmd, void const *data, unsigned len, void *context)
{
	bool worked = false ;
	if (sldev && !waiting) {
		worked = waiting = sldev->send(cmd,data,len,context);
		if (waiting)
			wait_time.restart();
	} else {
		fprintf(stderr, "Attempt to send on busy device\n");
	}
	return worked ;
}

static QString const empty_string ;

void rfidReader_t::timeout(void)
{
	if (waiting && (2000 < wait_time.elapsed())) {
		printf( "timeout waiting for response\n");
		waiting = false ;
	}
	if (!waiting)
	    send(sldev->SL_SELECTCARD,0,0,this);
}
