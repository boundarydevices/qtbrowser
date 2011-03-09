#ifndef __ACCELINPUT_H__
#define __ACCELINPUT_H__
/*
 * This header file declares the accelInput_t class for use in
 * reading accelerometer. Attempts have been made to try to
 * map this to the Acceleration portion of the "Device Motion"
 * events from the HTML5 specs:
 *
 *	http://dev.w3.org/geo/api/spec-source-orientation.html#motion_desc
 */

#include <QtGui>
#include <QLinkedList>

class accelInput_t : public QObject {
	Q_OBJECT
public:
	accelInput_t();
	~accelInput_t();

	Q_PROPERTY(double x READ getX);
	Q_PROPERTY(double y READ getY);
	Q_PROPERTY(double z READ getZ);

signals:
	void accelchange(double x, double y, double z);

private slots:
	void readData(int fd);

private:
	double getX(void) const { return x ; }
	double getY(void) const { return y ; }
	double getZ(void) const { return z ; }
	QSocketNotifier		*dev ;
	double			x ;
	double			y ;
	double			z ;
};

#endif
