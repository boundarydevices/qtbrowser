/*
 * This module file defines the methods of the kbdInput_t class for use in 
 * reading Linux input events
 */

#include <QApplication>
#include "kbdInput.h"
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <linux/input.h>
#include <dirent.h>
#include <sys/ioctl.h>

kbdInput_t::kbdInput_t()
	: lastKey(-1)
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
		char key_bitmask[(KEY_MAX+7)/8];
		memset(key_bitmask, 0, sizeof(key_bitmask));
		if (ioctl(fd, EVIOCGBIT(EV_KEY, sizeof(key_bitmask)), key_bitmask) >= 0) {
                        QSocketNotifier *dev = new QSocketNotifier(fd,dev->Read);
                        devs.push_back(dev);
			QObject::connect(dev, SIGNAL(activated(int)), this, SLOT(readKeycode(int)));
			continue;
		} else
			perror(devname);
		close(fd);
	}
	else
		perror(devname);
    }
    closedir(dir);
#if 0
        int fd = open(qPrintable(devname),O_RDONLY|O_NONBLOCK);
	if (0 <= fd) {
		dev = new QSocketNotifier(fd,dev->Read);
		printf( "%s: opened %s\n", __func__, qPrintable(devname));
                QObject::connect(dev, SIGNAL(activated(int)), this, SLOT(readKeycode(int)));
		return ;
	}
	perror(qPrintable(devname));
#endif
}

kbdInput_t::~kbdInput_t()
{
	while (!devs.empty()) {
                QSocketNotifier *dev = devs.back();
		devs.pop_back();
		if (0 < dev->socket()) {
			dev->setEnabled(false);
			close(dev->socket());
			delete dev ;
		}
	}
}

void kbdInput_t::readKeycode(int fd)
{
	struct ::input_event buffer[32];
	int n ;
	while (0 <= (n=::read(fd,buffer,sizeof(buffer)))) {
		n /= sizeof(buffer[0]);
		for (int i = 0 ; i < n ; i++) {
			if ((EV_KEY == buffer[i].type) 
			    &&
			    (0 == buffer[i].value)){ /* Key up */
				lastKey = buffer[i].code ;
				emit keyChanged(lastKey);
                                if (1 == lastKey) {
					printf( "------------ escape key release\n");
					QApplication::instance()->exit(0);
				}
			}
		}
	}

}

int kbdInput_t::getLastKey(void)
{
	return lastKey ;
}

void kbdInput_t::clearLastKey(void)
{
	lastKey = -1 ;
}

