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

class rfidReader_t : public QObject {
	Q_OBJECT  
public:
	rfidReader_t();
	~rfidReader_t();

	Q_PROPERTY(bool present READ isPresent);

	bool isPresent(void) const ;

	Q_INVOKABLE bool send(int, QString);

signals:
	void cardPresent(QString serial,QString cardType);
	void cardRemoved();

private slots:
	void readData(int fd);
	void timeout(void);

private:
	bool send(stronglink_t::command_e, void const *data, unsigned len, void *context);
        QSocketNotifier *notifier ;
	stronglink_t	*sldev ;
	bool		 waiting ;
	bool		 ispresent ;
	QTime		 wait_time ;
        QTimer		 timer ;
};

#endif
