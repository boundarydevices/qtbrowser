#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <sys/poll.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <ctype.h>

static void setBaud (int fd)
{
	struct termios oldState ;
	tcgetattr(fd,&oldState);

	struct termios newState = oldState;
	newState.c_cc[VMIN] = 1;

	cfsetispeed(&newState,B19200);
	cfsetospeed(&newState,B19200);
	newState.c_cc[VTIME] = 0;
	newState.c_cflag &= ~(PARENB|CSTOPB|CSIZE|CRTSCTS); 	// no parity
	newState.c_cflag |= (CLOCAL | CREAD |CS8);		// Select 8 data bits
	newState.c_lflag &= ~(ICANON | ECHO );				 // set raw mode for input
	newState.c_iflag &= ~(IXON | IXOFF | IXANY|INLCR|ICRNL|IUCLC);	 //no software flow control
	newState.c_oflag &= ~OPOST;			 //raw output
	tcsetattr( fd, TCSANOW, &newState );
}

int main( int argc, char const * const argv[] )
{
	if ( 1 < argc ) {
		char const *const deviceName = argv[1];
		int const fdSerial = open( deviceName, O_RDWR );
		if ( 0 <= fdSerial ) {
			fcntl( fdSerial, F_SETFD, FD_CLOEXEC );
			fcntl( fdSerial, F_SETFL, O_NONBLOCK );
			setBaud(fdSerial);
	
			struct termios oldStdinState ;
			tcgetattr(0,&oldStdinState);

			struct termios newState = oldStdinState;
			newState.c_lflag &= ~(ICANON|ECHO|ECHOE|ISIG);				 // set raw mode for input
			newState.c_iflag &= ~(IXON|IXOFF|IXANY|INLCR|ICRNL|IUCLC);	 //no software flow control, no signal handling
			newState.c_oflag &= ~OPOST;			 //raw output
			tcsetattr (0, TCSANOW, &newState);

			pollfd fds[2]; 
			fds[0].fd = fdSerial ;
			fds[0].events = POLLIN | POLLERR ;
			fds[1].fd = fileno(stdin);
			fds[1].events = POLLIN | POLLERR ;

			bool terminated = true ;
			bool doExit = false ;
			while ( !doExit ) {
				int const numReady = ::poll (fds, 2, 100);
				if ( 0 < numReady ) {
					for ( unsigned i = 0 ; i < 2 ; i++ ) {
						if ( fds[i].revents & POLLIN ) {
							char inBuf[80];
							int numRead = read( fds[i].fd, inBuf, sizeof(inBuf) );
							if (0 > numRead) {
								doExit = 1 ;
								break;
							}
							if (0 == i) {
								if (0 < numRead) {
									for ( int j = 0 ; j < numRead ; j++ ) {
										char const c = inBuf[j];
										if ( isprint(c) || ('\n' == c))
											printf( "%c", c );
										else
											printf( "[%02x]", (unsigned char)c );
									}
									terminated = ('\n' == inBuf[numRead-1]);
									fflush(stdout);
								}
							} else {
								for (unsigned i = 0 ; i < numRead; i++) {
									if ('\x03' == inBuf[i]) {
										doExit = 1 ;
										break;
									}
								}
								if (!doExit) {
									write (fdSerial,inBuf,numRead);
								} else
									break;
							}

						}
					}
				} else if (!terminated) {
					printf("\r\n"); fflush(stdout);
					terminated = true ;
				}
			}
			tcsetattr (0, TCSANOW, &oldStdinState);
			close( fdSerial );
		}
		else
			perror( deviceName );
	}
	else
		fprintf( stderr, "Usage: setBaud /dev/ttyS0 baudRate\n" );
	return 0 ;
}
