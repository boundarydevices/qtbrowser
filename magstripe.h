#ifndef __MAGSTRIPE_H__
#define __MAGSTRIPE_H__

/*
 * This header file declares the magstripe_t class for use in 
 * reading magnetic stripe data from a Neuron MCR-37O-T as
 * described in 
 *	http://boundarydevices.com/blogs/magnetic-stripe-driver-for-linux-gpio
 */
 
#include <QtGui>
#include <QLinkedList>

class magstripe_t : public QObject {
	Q_OBJECT  
public:
	magstripe_t();
	~magstripe_t();

	Q_PROPERTY(QString magstripe READ getMagstripe);
	QString getMagstripe(void);

	Q_PROPERTY(QString switchState READ getSwitchState);
	QString getSwitchState(void);
	
	Q_INVOKABLE void clear();

signals:
	void switchChange(QString);
	void swipe(QString);

private slots:
	void readData(int fd);

private:
        QLinkedList<QSocketNotifier *> 	devs ;
	bool			     	front_closed ;
	bool				rear_closed ;
	QString		 	     	magstripe ;
	QString			     	parsing ;
	char				switchStates[26];
};

#endif
