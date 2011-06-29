#ifndef __PRINT_H__
#define __PRINT_H__
/*
 * This header file declares the printer class, which allows
 * an application to print an image.
 *
 */
#include <QtGui>
#include <QLinkedList>
#include <QWebView>
#include <QPixmap>

class printer_t : public QObject {
	Q_OBJECT
public:
	printer_t();
	~printer_t();

	Q_INVOKABLE bool print(QPixmap const &);

private:
};

#endif
