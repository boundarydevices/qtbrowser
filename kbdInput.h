#ifndef __KBDINPUT_H__
#define __KBDINPUT_H__
/*
 * This header file declares the kbdInput_t class for use in 
 * reading Linux input events
 */
 
#include <QtGui>
#include <QLinkedList>

class kbdInput_t : public QObject {
	Q_OBJECT  
public:
	kbdInput_t();
	~kbdInput_t();

	Q_PROPERTY(int lastKey READ getLastKey);

	int getLastKey(void);

	Q_INVOKABLE void clearLastKey();

signals:
	void keyChanged(int newValue);

private slots:
	void readKeycode(int fd);

private:
        QLinkedList<QSocketNotifier *> devs ;
	int		 	     lastKey ;
};

#endif
