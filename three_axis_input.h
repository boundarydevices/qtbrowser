#ifndef __THREE_AXIS_INPUT_H__
#define __THREE_AXIS_INPUT_H__
/*
 * This header file declares the three_axis_input class for use in
 * reading accelerometer and compass data. Attempts have been made to try to
 * map this to the Acceleration portion of the "Device Motion"
 * events from the HTML5 specs:
 *
 *	http://dev.w3.org/geo/api/spec-source-orientation.html#motion_desc
 */

#include <QtGui>
#include <QLinkedList>

class three_axis_input_t : public QObject {
	Q_OBJECT
public:
	three_axis_input_t(char const *labelfrag);
	~three_axis_input_t();

	Q_PROPERTY(int x READ getX);
	Q_PROPERTY(int y READ getY);
	Q_PROPERTY(int z READ getZ);

	Q_PROPERTY(int xmax READ getX_max);
	Q_PROPERTY(int ymax READ getY_max);
	Q_PROPERTY(int zmax READ getZ_max);
	
	Q_PROPERTY(int xmin READ getX_min);
	Q_PROPERTY(int ymin READ getY_min);
	Q_PROPERTY(int zmin READ getZ_min);

signals:
	void axis_change(double x, double y, double z);

private slots:
	void readData(int fd);

private:
	int getX(void) const { return x ; }
	int getY(void) const { return y ; }
	int getZ(void) const { return z ; }
	int getX_max(void) const { return xmax ; }
	int getY_max(void) const { return ymax ; }
	int getZ_max(void) const { return zmax ; }
	int getX_min(void) const { return xmin ; }
	int getY_min(void) const { return ymin ; }
	int getZ_min(void) const { return zmin ; }
	QSocketNotifier		*dev ;
	int			xmax ;
	int			ymax ;
	int			zmax ;
	int			xmin ;
	int			ymin ;
	int			zmin ;
	int			x ;
	int			y ;
	int			z ;
};

#endif
