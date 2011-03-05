#ifndef __RFID_H__
#define __RFID_H__
/*
 * This header file declares the rfidReader_t class, which 
 * provides Qt bindings for the stronglink_t Proximity smart
 * card reader class.
 *
 */
 
#include <QtGui>
#include <QLinkedList>
#include "stronglink.h"
#include <QTimer>

class rfid_message_t {
public:
	rfid_message_t(int code, void const *data, unsigned len );
	rfid_message_t(int code, QByteArray const &data );

	int code_ ;		// result in returned messages
	QByteArray data_ ;
};

typedef QLinkedList<rfid_message_t> rfid_message_list_t ;
typedef QLinkedList<rfid_message_list_t> rfid_message_queue_t ;

class rfidReader_t : public QObject {
	Q_OBJECT  
public:
	rfidReader_t();
	~rfidReader_t();

	Q_PROPERTY(bool present READ isPresent);
	bool isPresent(void) const ;

	Q_PROPERTY(bool busy READ isBusy);
	bool isBusy(void) const ;

	Q_PROPERTY(int elapsed READ getElapsed);
	int getElapsed(void) const ;

	Q_PROPERTY(int ticks READ getTicks);
	int getTicks(void) const ;

	Q_INVOKABLE bool send(int command, QByteArray data);
	Q_INVOKABLE bool send(QVariantList data);

signals:
	void cardPresent(QString serial,QString cardType);
	void cardRemoved();
	void response(int result, QVariantList);

private slots:
	void readData(int fd);
	void timerEvent(QTimerEvent *);

private:
	bool send();
	bool send(stronglink_t::command_e, void const *data, unsigned len, void *context);
	QSocketNotifier        *notifier ;
	stronglink_t	       *sldev ;
	bool		 	waiting ;
	bool		 	ispresent ;
	QTime		 	wait_time ;
	rfid_message_queue_t	requests_ ;
	rfid_message_list_t	responses_ ;
	int			ticks_ ;
};

#endif
