/*
 * This module file defines the methods of the magstripe_t class for use in 
 * reading mag stripe data
 */
 
#include "magstripe.h"
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <linux/input.h>
#include <dirent.h>
#include <sys/ioctl.h>
#include <ctype.h>

magstripe_t::magstripe_t()
	: front_closed(false)
        , rear_closed(false)
        , magstripe("")
	, parsing("")
{
	printf("in %s\n", __func__ );
	memset(switchStates,0,sizeof(switchStates));

	char const *devname=getenv("MAGSTRIPEDEV");
	if (0 == devname)
		devname = "/dev/magstripe" ;
	int fd = open(devname,O_RDONLY|O_NONBLOCK);
	if (0 <= fd) {
		QSocketNotifier *dev = new QSocketNotifier(fd,dev->Read);
		devs.push_back(dev);
		QObject::connect(dev, SIGNAL(activated(int)), this, SLOT(readData(int)));
		printf( "opened %s\n", devname);
	}
	else
		perror(devname);
}

magstripe_t::~magstripe_t()
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

void magstripe_t::readData(int fd)
{
	char buffer[32];
	int n ;
	while (0 < (n=::read(fd,buffer,sizeof(buffer)))) {
		buffer[n] = '\0' ;
       		for (int i = 0 ; i < n ; i++) {
			char c = buffer[i];
			if (iscntrl(c)) {
				if (1 == parsing.length()) {
					c = parsing.at(0).toLatin1();
					if (isalpha(c))
						switchStates[toupper(c)-'A']=c;
					emit switchChange(parsing);
				} else if (0 < parsing.length()) {
					magstripe = parsing ;
					emit swipe(magstripe);
				}
				parsing.clear();
			} else
				parsing += c ;
		}
	}
}

QString magstripe_t::getMagstripe(void)
{
	printf ("%s: %s\n", __func__, magstripe.toUtf8().data() );
	return magstripe ;
}

QString magstripe_t::getSwitchState(void)
{
	printf ("in %s\n", __func__ );
	QString s ;
	for (unsigned i = 0 ; i < sizeof(switchStates); i++) {
		char c = switchStates[i];
		if(c)
			s += c ;
	}
	return s ;
}

void magstripe_t::clear(void)
{
	magstripe.clear();
}

