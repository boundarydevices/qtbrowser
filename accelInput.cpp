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
	, xmax(0)
	, ymax(0)
	, zmax(0)
	, xmin(0)
	, ymin(0)
	, zmin(0)
	, x(0)
	, y(0)
	, z(0)
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
	printf( "%s: try %s\n", __func__, filename);
	int fd = open(devname, O_RDONLY|O_NONBLOCK);
	if (0 <= fd) {
		char abs_bitmask[(ABS_MAX+7)/8];
		memset(abs_bitmask, 0, sizeof(abs_bitmask));
		if ((ioctl(fd, EVIOCGBIT(EV_ABS, sizeof(abs_bitmask)), abs_bitmask) >= 0)
		    &&
		    (0 != (abs_bitmask[ABS_Z/8]&(1<<(ABS_Z&7))))) {
			char name[256]= "Unknown";
			int namelen ;
			if((namelen=ioctl(fd, EVIOCGNAME(sizeof(name)), name)) >= 0) {
				struct input_absinfo xinfo ;
				struct input_absinfo yinfo ;
				struct input_absinfo zinfo ;
				name[namelen] = 0 ;
				if (strcasestr(name,"ccel")) {
					if ((ioctl(fd, EVIOCGABS(ABS_X), &xinfo) >= 0)
					    &&
					    (ioctl(fd, EVIOCGABS(ABS_Y), &yinfo) >= 0)
					    &&
					    (ioctl(fd, EVIOCGABS(ABS_Z), &zinfo) >= 0)){
						dev = new QSocketNotifier(fd,dev->Read);
						QObject::connect(dev, SIGNAL(activated(int)), this, SLOT(readData(int)));
						xmax=xinfo.maximum ;
						ymax=yinfo.maximum ;
						zmax=zinfo.maximum ;
						xmin=xinfo.minimum ;
						ymin=yinfo.minimum ;
						zmin=zinfo.minimum ;
			printf ("%s: connected on %s\n", __func__, filename );
			printf("%s: abs_x: %d..%d\n", __func__, xmin, xmax);
			printf("%s: abs_y: %d..%d\n", __func__, ymin, ymax);
			printf("%s: abs_z: %d..%d\n", __func__, zmin, zmax);
						break ;
				}
				else
					printf( "%s: %s: not accelerometer\n", devname, name);
				} else
					perror(devname);
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
					x = event.value ;
				} else if (ABS_Y == event.code) {
					y = event.value ;
				} else if (ABS_Z == event.code) {
					z = event.value ;
				} else {
					printf ("unknown ABS code %d\n", event.code);
				}
			} else if (EV_SYN == event.type) {
				emit accelchange(1.0*x,1.0*y,1.0*z);
			}
		}
	}
}
