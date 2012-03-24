/*
 * This module file defines the methods of the accelInput_t class for use in
 * reading Linux input events
 */

#include "accelInput.h"
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <linux/input.h>
#include <dirent.h>
#include <sys/ioctl.h>

accelInput_t::accelInput_t()
	: three_axis_input_t("cceler")
{
}

accelInput_t::~accelInput_t()
{
}


