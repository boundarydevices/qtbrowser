#ifndef __COMPASSINPUT_H__
#define __COMPASSINPUT_H__
/*
 * This header file declares the compassInput_t class for use in
 * reading compasserometer. Attempts have been made to try to
 * map this to the Acceleration portion of the "Device Motion"
 * events from the HTML5 specs:
 *
 *	http://dev.w3.org/geo/api/spec-source-orientation.html#motion_desc
 */

#include "three_axis_input.h"

class compassInput_t : public three_axis_input_t {
	Q_OBJECT
public:
	compassInput_t();
	~compassInput_t();
};

#endif
