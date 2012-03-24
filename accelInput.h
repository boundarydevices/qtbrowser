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

#include "three_axis_input.h"

class accelInput_t : public three_axis_input_t {
	Q_OBJECT
public:
	accelInput_t();
	~accelInput_t();
};

#endif
