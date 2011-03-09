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
	: dev(0)
{
    char devname[PATH_MAX];
    char *filename;
    DIR *dir;
    struct dirent *de;
    char const dirname[] = {
            "/dev/input"
    };
    dir = opendir(dirname);
    if(dir == NULL)
        return ;
    strcpy(devname, dirname);
    filename = devname + strlen(devname);
    *filename++ = '/';
    while((de = readdir(dir))) {
        if(de->d_name[0] == '.' &&
           (de->d_name[1] == '\0' ||
            (de->d_name[1] == '.' && de->d_name[2] == '\0')))
            continue;
        strcpy(filename, de->d_name);
	int fd = open(devname, O_RDONLY|O_NONBLOCK);
	if (0 <= fd) {
		char abs_bitmask[(ABS_MAX+7)/8];
		memset(abs_bitmask, 0, sizeof(abs_bitmask));
		if ((ioctl(fd, EVIOCGBIT(EV_ABS, sizeof(abs_bitmask)), abs_bitmask) >= 0)
		    &&
		    (0 != (abs_bitmask[ABS_Z/8]&(1<<(ABS_Z&7))))) {
			struct input_absinfo xinfo ;
			struct input_absinfo yinfo ;
			struct input_absinfo zinfo ;
			if ((ioctl(fd, EVIOCGABS(ABS_X), &xinfo) >= 0)
			    &&
			    (ioctl(fd, EVIOCGABS(ABS_Y), &yinfo) >= 0)
			    &&
			    (ioctl(fd, EVIOCGABS(ABS_Z), &zinfo) >= 0)){
				dev = new QSocketNotifier(fd,dev->Read);
				QObject::connect(dev, SIGNAL(activated(int)), this, SLOT(readData(int)));
	printf ("%s: connected on %s\n", __func__, filename );
	printf("%s: abs_x: %d..%d\n", __func__, xinfo.minimum, xinfo.maximum);
	printf("%s: abs_y: %d..%d\n", __func__, yinfo.minimum, yinfo.maximum);
	printf("%s: abs_z: %d..%d\n", __func__, zinfo.minimum, zinfo.maximum);
				break ;
			} else
				perror(devname);
		} else
			perror(devname);
		close(fd);
	}
	else
		perror(devname);
    }
    closedir(dir);
}

accelInput_t::~accelInput_t()
{
}

void accelInput_t::readData(int fd)
{
	struct ::input_event buffer[32];
	int n ;
	while (0 <= (n=::read(fd,buffer,sizeof(buffer)))) {
		n /= sizeof(buffer[0]);
		for (int i = 0 ; i < n ; i++) {
			struct input_event const &event = buffer[i];
			if (EV_ABS == event.type) {
				if (ABS_X == event.code) {
					x = ((double)event.value)*0.047 ;
				} else if (ABS_Y == event.code) {
					y = ((double)event.value)*0.047 ;
				} else if (ABS_Z == event.code) {
					z = ((double)event.value)*0.047 ;
				} else {
					printf ("unknown ABS code %d\n", event.code);
				}
			} else if (EV_SYN == event.type) {
				emit accelchange(x,y,z);
			}
		}
	}
}
