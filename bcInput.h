#ifndef __BCINPUT_H__
#define __BCINPUT_H__
/*
 * This header file declares the bcInput_t class for use in 
 * reading barcode scanner data
 */
 
#include <QtGui>
#include <QLinkedList>

class bcInput_t : public QObject {
	Q_OBJECT  
public:
	bcInput_t();
	~bcInput_t();

	Q_PROPERTY(QString barcode READ getBarcode);

	QString getBarcode(void);

	Q_INVOKABLE void clear();

signals:
	void scanned(QString);

private slots:
	void readData(int fd);

private:
        QLinkedList<QSocketNotifier *> devs ;
	QString		 	     barcode ;
	QString			     parsing ;
	char const		    *exitbc ;
};

#endif
