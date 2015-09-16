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
#include <assert.h>
#include <QMap>

rfid_message_t::rfid_message_t(int code, void const *data, unsigned len )
	: code_(code)
	, data_((char const *)data,len)
{
}

rfid_message_t::rfid_message_t(int code, QByteArray const &data)
	: code_(code)
	, data_(data)
{
}

rfidReader_t::rfidReader_t()
	: notifier(0)
	, sldev(0)
	, waiting(false)
	, ispresent(false)
{
	char const *devname = getenv("RFIDDEV");
	if (devname) {
		sldev = new stronglink_t(devname);
		if (sldev->isOpen()) {
                        notifier = new QSocketNotifier(sldev->getFd(),notifier->Read);
			QObject::connect(notifier, SIGNAL(activated(int)), this, SLOT(readData(int)));
			startTimer(100);
		} else {
			delete sldev ; sldev = 0 ;
		}
	}
}

rfidReader_t::~rfidReader_t()
{
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

bool rfidReader_t::isBusy(void) const {
	return waiting ;
}

int rfidReader_t::getElapsed(void) const {
	return wait_time.elapsed();
}

int rfidReader_t::getTicks(void) const {
	return ticks_ ;
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
static QString const resultName("result");
static QString const dataName("data");
static QString const codeName("code");

static QString const empty_string ;
static char const hexDigits[17] = "0123456789ABCDEF";
static QString encode(void const *data, unsigned len)
{
	if (0 == len)
		return empty_string ;
	QString s ; s.reserve(3*len+1);
	char const *bytes = (char const *)data;
	while (len--) {
		char const c = *bytes++ ;
		if (isalnum(c)) {
			s += c ;
		} else {
		    s += '%';
		    s += hexDigits[c>>4];
		    s += hexDigits[c&0x0f];
		}
	}
	return s ;
}

static void build_response( rfid_message_list_t const &msgs,
			    QVariantList 	      &vl)
{
        rfid_message_list_t::const_iterator msgit ;
	for (msgit = msgs.begin(); msgit != msgs.end(); msgit++) {
                QMap<QString, QVariant> map ;
		map[resultName] = QVariant((*msgit).code_);
		map[dataName] = QVariant(encode((*msgit).data_.constData(),(*msgit).data_.length()));
		vl.push_back(QVariant(map));
	}
}

void rfidReader_t::readData(int fd)
{
	if (notifier && sldev) {
		enum stronglink_t::response_e resp ;
		void const *rxdata ;
		unsigned rxlen ;
		void *context ;
		while (sldev->receive(resp,rxdata,rxlen,context)) {
			waiting = false ;
			long long serial ;
			bool nowPresent = sldev->serialNumber(serial);
			if (context) {
				assert (!requests_.empty());
				assert (!responses_.empty());
				rfid_message_t &r = responses_.back();
				r.code_ = resp ;
				r.data_ = QByteArray((char const *)rxdata,rxlen);
				rfid_message_list_t &trans = requests_.front();
				if (trans.empty() || (0 != r.code_)) {
					requests_.pop_front();
					QVariantList v ;
					build_response(responses_,v);
					responses_.clear();
					emit response(resp,v);
				} else {
					send();
				}
			}
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
	} else {
		printf( "No notifier %p or dev %p\n", notifier, sldev);
	}
}

bool rfidReader_t::send(int cmd, QByteArray data)
{
	rfid_message_t msg(cmd,data);
	rfid_message_list_t list ; list.push_back(msg);
	requests_.push_back(list);
	return send();
}

static bool build_message_list( QVariantList const &vl,
			        rfid_message_list_t &msgs)
{
	QVariantList::const_iterator it ;

	for (it = vl.begin() ; it != vl.end(); it++) {
		QVariant const &el = (*it);
		if (QVariant::Map == el.type()) {
			QVariantMap v = el.toMap();
			QVariantMap::const_iterator codeit = v.find(codeName);
			QVariantMap::const_iterator datait = v.find(dataName);
			if (codeit != v.end()) {
				QString data = (datait != v.end())
						? (*datait).toString()
					        : empty_string;
				msgs.push_back(rfid_message_t((*codeit).toInt(),data.toLatin1()));
			} else {
				printf( "message is missing code\n");
				return false ;
			}
		} else {
			printf( "Invalid message: use form: { code:0x01 [, data:'\x01\x02']}\n");
			return false ;
		}
	}
	return !msgs.empty();
}

bool rfidReader_t::send(QVariantList requests)
{
	rfid_message_list_t msgs ;
	if (build_message_list(requests,msgs)) {
		requests_.push_back(msgs);
		return send();
	} else
		printf ("Error decoding message list\n");
	return false ;
}

bool rfidReader_t::send()
{
	if (sldev && !waiting) {
		if (!requests_.empty()) {
                        rfid_message_list_t &list = requests_.front();
			if (!list.empty()) {
				rfid_message_t &msg = list.front();
                                if (send((stronglink_t::command_e)msg.code_, msg.data_.constData(), msg.data_.length(), &msg )) {
					responses_.push_back(list.front());
					list.pop_front();
				}
				else
					printf( "send error\n");
			}
			else
				printf("empty list in send\n");
		}
		else
			printf("Nothing to send\n");
	}
	return false ;
}

bool rfidReader_t::send(stronglink_t::command_e cmd, void const *data, unsigned len, void *context)
{
	bool worked = false ;
	if (sldev && !waiting) {
		worked = waiting = sldev->send(cmd,data,len,context);
		if (waiting) {
                        wait_time.restart();
		} else
			printf( "sldev send error\n");
	} else {
		fprintf(stderr, "Attempt to send on busy device\n");
	}
	return worked ;
}

void rfidReader_t::timerEvent(QTimerEvent *)
{
	if (waiting && (2000 < wait_time.elapsed())) {
		printf( "timeout waiting for response\n");
		waiting = false ;
	}
	if (!waiting){
		if (requests_.empty()) {
			send(sldev->SL_SELECTCARD,0,0,0);
		} else {
			send();
		}
	}
	++ticks_ ;
}
