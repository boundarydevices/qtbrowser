#ifndef __GPSINPUT_H__
#define __GPSINPUT_H__

/*
 * This header file declares the gpsInput_t class for use in
 * reading GPS sentences from an attached GPS receiver.
 */

#include <QtGui>
#include <QLinkedList>

class gpsInput_t : public QObject {
	Q_OBJECT
public:
	gpsInput_t();
	~gpsInput_t();

signals:
	void sentence(QString);

private slots:
	void readData(int fd);

private:
        QSocketNotifier	*dev ;
	QString		parsing ;
};

#endif
