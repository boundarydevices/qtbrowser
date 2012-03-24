/*
 * This module file defines the methods of the compassInput_t class for use in
 * reading Linux input events
 */

#include "compassInput.h"
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <linux/input.h>
#include <dirent.h>
#include <sys/ioctl.h>

compassInput_t::compassInput_t()
	: three_axis_input_t("ompass")
{
}

compassInput_t::~compassInput_t()
{
}


