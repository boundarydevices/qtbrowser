/*
 * This module file defines the methods of the bcInput_t class for use in 
 * reading Linux input events
 */
 
#include "bcInput.h"
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <linux/input.h>
#include <dirent.h>
#include <sys/ioctl.h>

#include <termios.h>

static unsigned const _standardBauds[] = {
   0,
   50,
   75,
   110,
   134,
   150,
   200,
   300,
   600,
   1200,
   1800,
   2400,
   4800,
   9600,
   19200,
   38400
};
static unsigned const numStandardBauds = sizeof( _standardBauds )/sizeof( _standardBauds[0] );

static unsigned const _highSpeedMask = 0010000 ;
static unsigned const _highSpeedBauds[] = {
   0,
   57600,  
   115200, 
   230400, 
   460800, 
   500000, 
   576000, 
   921600, 
   1000000,
   1152000,
   1500000,
   2000000,
   2500000,
   3000000,
   3500000,
   4000000 
};

static unsigned const numHighSpeedBauds = sizeof( _highSpeedBauds )/sizeof( _highSpeedBauds[0] );

static bool baudRateToConst( unsigned bps, unsigned &constant )
{
   unsigned baudIdx = 0 ;
   bool haveBaud = false ;
   
   unsigned i ;
   for( i = 0 ; i < numStandardBauds ; i++ )
   {
      if( _standardBauds[i] == bps )
      {
         haveBaud = true ;
         baudIdx = i ;
         break;
      }
   }
   
   if( !haveBaud )
   {
      for( i = 0 ; i < numHighSpeedBauds ; i++ )
      {
         if( _highSpeedBauds[i] == bps )
         {
            haveBaud = true ;
            baudIdx = i | _highSpeedMask ;
            break;
         }
      }
   }

   constant = baudIdx ;
   
   return haveBaud ;
}

bcInput_t::bcInput_t()
	: barcode("")
	, parsing("")
	, exitbc(0)
{
	char *descr=getenv("BCDEV");
	if (descr) {
		descr = strdup(descr);
		char *cdevname=strtok(descr,":,");
		int fd = open(cdevname,O_RDONLY|O_NONBLOCK);
		if (0 <= fd) {
			char *cbaud = strtok(0,",");
			char *cdatabits = strtok(0,",");
			char *cparity = strtok(0,",");
			char *cstop = strtok(0,",");
			unsigned baud = cbaud ? strtoul(cbaud,0,0) : 9600 ;
			unsigned databits = cdatabits ? strtoul(cdatabits,0,0) : 8 ;
			unsigned parity = cparity ? *cparity : 'N' ;
			unsigned stopBits = cstop ? *cstop-'0' : 1 ;
			printf( "setup %s,%u,%u,%c,%u here\n", cdevname, baud, databits, parity,stopBits);

			/* set raw mode for keyboard input */
			struct termios newState ;
			tcgetattr(fd,&newState);
			newState.c_cc[VMIN] = 1;
			
			unsigned baudConst ;
			if( baudRateToConst(baud, baudConst ) ){
				cfsetispeed(&newState, baudConst);
				cfsetospeed(&newState, baudConst);
			}
			
			//
			// Note that this doesn't appear to work!
			// Reads always seem to be terminated at 16 chars!
			//
			newState.c_cc[VTIME] = 0; // 1/10th's of a second, see http://www.opengroup.org/onlinepubs/007908799/xbd/termios.html
			newState.c_cflag &= ~(PARENB|CSTOPB|CSIZE|CRTSCTS);  // Mask character size to 8 bits, no parity, Disable hardware flow control
			
			if( 'E' == parity )
			{
				newState.c_cflag |= PARENB ;
				newState.c_cflag &= ~PARODD ;
			}
			else if( 'O' == parity )
			{
				newState.c_cflag |= PARENB | PARODD ;
			}
			else if( 'S' == parity )
			{
				newState.c_cflag |= PARENB | IGNPAR | CMSPAR ;
				newState.c_cflag &= ~PARODD ;
			}
			else if( 'M' == parity )
			{
				newState.c_cflag |= PARENB | IGNPAR | CMSPAR | PARODD ;
			}
			else {
			} // no parity... already set
			
			newState.c_cflag |= (CLOCAL | CREAD |CS8);                       // Select 8 data bits
			if( 7 == databits ){
				newState.c_cflag &= ~CS8 ;
			}
			
			if( 1 != stopBits )
				newState.c_cflag |= CSTOPB ;
			
			newState.c_lflag &= ~(ICANON | ECHO );                           // set raw mode for input
			newState.c_iflag &= ~(IXON | IXOFF | IXANY|INLCR|ICRNL|IUCLC);   //no software flow control
			newState.c_oflag &= ~OPOST;                      //raw output
			tcsetattr( fd, TCSANOW, &newState );
			
                        QSocketNotifier *dev = new QSocketNotifier(fd,dev->Read);
                        devs.push_back(dev);
			QObject::connect(dev, SIGNAL(activated(int)), this, SLOT(readData(int)));

                        exitbc=getenv("BCEXIT");
			if (exitbc) {
				printf( "have exit barcode %s\n", exitbc);
				exitbc=strdup(exitbc);
				printf( "now exit barcode %s(%p)\n", exitbc,exitbc);
			}
		}
		else
			perror(cdevname);
		free(descr);
	}
}

bcInput_t::~bcInput_t()
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
	if (exitbc) {
		free((void *)exitbc);
	}
}

void bcInput_t::readData(int fd)
{
	char buffer[32];
	int n ;
	while (0 <= (n=::read(fd,buffer,sizeof(buffer)))) {
		buffer[n] = '\0' ;
		for (int i = 0 ; i < n ; i++) {
			char c = buffer[i];
			if (iscntrl(c)) {
				if (0 < parsing.length()) {
					barcode = parsing ;
					printf( "barcode %s\n", barcode.toLocal8Bit().data());
					printf( "exit %s(%p)\n", exitbc, exitbc);
					if ((0 != exitbc) && (0 == strcasecmp(exitbc,barcode.toLocal8Bit().data()))) {
						exit(0);
					} else {
						emit scanned(barcode);
						parsing.clear();
					}
				}
			} else
				parsing += c ;
		}
	}
}

QString bcInput_t::getBarcode(void)
{
	return barcode ;
}

void bcInput_t::clear(void)
{
	barcode.clear();
}

