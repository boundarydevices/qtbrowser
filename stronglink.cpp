/*
 * Module stronglink.cpp
 * 
 * This module defines the methods of the stronglink_t class as
 * declared in stronglink.h.
 *
 */

#include "stronglink.h"
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <assert.h>
#include <string.h>

// #define DEBUG_MSGS
#define NOCARDSERIAL	0LL

#define REQUEST_TAG '\xBA'
#define RESPONSE_TAG '\xBD'

stronglink_t::stronglink_t (char const *devname)
	: fd_(open(devname, O_RDWR|O_NONBLOCK))
	, len_(0)
	, prev_command_(SL_RESET)
	, serial_(NOCARDSERIAL)
{
	if (!isOpen()) {
		perror(devname);
		return ;
	}
	fcntl(fd_, F_SETFD, FD_CLOEXEC );
	
	struct termios newState ;
	tcgetattr(fd_,&newState);
	cfmakeraw( &newState );
	newState.c_cc[VMIN] = 1;
	cfsetispeed(&newState, B115200);
	cfsetospeed(&newState, B115200);
	newState.c_cc[VTIME] = 0; // 1/10th's of a second, see http://www.opengroup.org/onlinepubs/007908799/xbd/termios.html
	newState.c_cflag &= ~(PARENB|CSTOPB|CSIZE|CRTSCTS);  // Mask character size to 8 bits, no parity, Disable hardware flow control
	newState.c_cflag |= (CLOCAL | CREAD |CS8);                       // Select 8 data bits
	newState.c_lflag &= ~(ICANON | ECHO );                           // set raw mode for input
	newState.c_iflag &= ~(IXON | IXOFF | IXANY|INLCR|ICRNL|IUCLC);   //no software flow control
	newState.c_oflag &= ~OPOST;                      //raw output
	tcsetattr( fd_, TCSANOW, &newState );
}

stronglink_t::~stronglink_t(void)
{
	if (isOpen()) {
		close(fd_);
		fd_ = -1 ;
	}
}

static char checksum(enum stronglink_t::command_e cmd, void const *data, unsigned len)
{
	char const *bytes = (char const *)data ;
	char sum = REQUEST_TAG
		 ^ (char)(len+2)
		 ^ (char)cmd;
	while (len--)
		sum ^= *bytes++ ;
	return sum ;
}

bool stronglink_t::send (enum command_e cmd, void const *data, unsigned datalen, void *context)
{
	assert (0 == len_);
	assert (253 >= datalen);
	int xlen = datalen+2 ; // transmitted length includes command and checksum
	char buf[512];

	if (!isOpen())
		return false ;
	char *nextout = buf;
	*nextout++ = REQUEST_TAG ;
	*nextout++ = xlen ;
	*nextout++ = cmd ;
	if (0 < datalen) {
		memcpy(nextout,data,datalen);
		nextout += datalen ;
	}
	*nextout++ = checksum(cmd,data,datalen);
	xlen = nextout-buf ;
#ifdef DEBUG_MSGS
	printf( "tx: ");
	for (int i = 0 ; i < xlen ; i++) {
		printf( "%02x ", (unsigned)buf[i]);
	}
	printf("\n");
#endif
	if (xlen == write(fd_,buf,xlen)) {
		prev_command_ = cmd ;
		return true ;
	} else {
		return false ;
	}
}

bool stronglink_t::receive (enum response_e &value, void const *&data, unsigned &len, void *&context)
{
	int space = sizeof(buf_)-len_ ;
	assert (0 < space);
	int numRead = read(fd_,buf_+len_,space);
	if (0 < numRead) {
#ifdef DEBUG_MSGS
		printf( "rx: ");
		for (int i = 0 ; i < numRead ; i++) {
			printf("%02x ", (unsigned)buf_[len_+i]);
		}
		printf("\n");
#endif
		len_ += numRead ;
		while ((0 < len_) && (RESPONSE_TAG != buf_[0])) {
			fprintf (stderr, "%s: invalid response tag %02x, len %u\n", __func__, (unsigned)buf_[0], len_);
			memmove(buf_, buf_+1,len_-1);
			--len_ ;
		}
		if (3 < len_) {
			if (prev_command_ != buf_[2]) {
				fprintf (stderr,"%s: invalid response %02x, expecting %02x\n", 
					 __func__, 
					 (unsigned)buf_[2],
					 (unsigned)prev_command_ );
				len_ = 0 ;
				return false ;
			}
			int expectedLen = buf_[1];
			if (len_ >= expectedLen+2) {
				char sum = buf_[expectedLen+1];
				char calc = 0 ;
				for (int i = 0 ; i <= expectedLen ; i++) {
					calc ^= buf_[i];
				}
				len = expectedLen-3 ;
				if (len)
                                        data = buf_+ 4 ;
				else
					data = 0 ;
				
				len_ = 0 ;
#ifdef DEBUG_MSGS
				printf("%s: checksum %02x/%02x\n", __func__, sum, calc );
#endif
				if (calc == sum) {
					value = (response_e)buf_[3];
					if (SLSTAT_SUCCESS == value) {
						if ((SL_SELECTCARD == prev_command_) 
						    &&
						    (4 <= len)
						    &&
						    (sizeof(serial_)+1 >= len)) {
							serial_ = 0 ;
							memcpy(&serial_,data,len-1);
						}
					} else if (SLSTAT_NOCARD == value) {
						serial_ = NOCARDSERIAL ;
					}
					return true ;
				}
			}
		}
	}
	return false ;
}


#ifdef STRONGLINK_TEST
#include <sys/poll.h>
#include <sys/time.h>
#include <ctype.h>
#include <stdlib.h>

inline long long timeValToMs( struct timeval const &tv )
{
   return ((long long)tv.tv_sec*1000)+((long long)tv.tv_usec / 1000 );
}

inline long long tickMs()
{
   struct timeval now ; gettimeofday( &now, 0 );
   return timeValToMs( now );
}

#define DONTKNOW 	-1
#define CARDNOTPRESENT 	0
#define CARDPRESENT	1

static char const * const prompts[] = {
	"\nunknown> "
,	"\nno card> "
,	"\ncard present> "
};

static void prompt(int state){
	printf("%s", prompts[state+1]); fflush(stdout);
}


static char const *const cardTypeNames[] = {
   "None"
,  "Mifare Standard 1K card"
,  "Mifare Pro card"
,  "Mifare UltraLight card"
,  "Mifare Standard 4K card"
,  "Mifare ProX card"
,  "Mifare DesFire card"
};

#define ARRAYSIZE(__arr) (sizeof(__arr)/sizeof(__arr[0]))

static void unescape( char     *s,        // in+out
                      unsigned &length )  // out
{
   enum state_t {
      normal,
      backslash,
      hex1,
      hex2
   };

   char *const start = s ;
   char *nextOut = start ;
   char c ;
   state_t state = normal ;

   while( 0 != (c = *s++) ){
      switch( state ){
         case normal: {
            if( '\\' == c ){
               state = backslash ;
            }
            else 
               *nextOut++ = c ;
            break ;
         }
         case backslash: {
            if( 'n' == c ){
               *nextOut++ = '\n' ;
            }
            else if( 'r' == c ){
               *nextOut++ = '\r' ;
            }
            else if( '\\' == c ){
               *nextOut++ = '\\' ;
            }
            else if( 'x' == c ){
               state = hex1 ;
            }
            break ;
         }
         case hex1: {
            if( ('0' <= c) && ('9' >= c) )
               *nextOut = (c-'0')<< 4;
            else if( ('a' <= c) && ('f' >= c) )
               *nextOut = (c-'a'+10)<< 4;
            else if( ('A' <= c) && ('F' >= c) )
               *nextOut = (c-'A'+10)<< 4;
            else
               state = normal ;
            if( state != normal )
               state = hex2 ;
            break ;
         }
         case hex2: {
            if( ('0' <= c) && ('9' >= c) )
               *nextOut++ += c-'0' ;
            else if( ('a' <= c) && ('f' >= c) )
               *nextOut++ += c-'a'+10 ;
            else if( ('A' <= c) && ('F' >= c) )
               *nextOut++ += c-'a'+10 ;
            state = normal ;
            break ;
         }
      }
   }

   length = nextOut-start ;
}

static void process_command(stronglink_t &sldev, char *cmd, int cmdlen)
{
	char *end = cmd+cmdlen ;
	while ((end > cmd) && iscntrl(end[-1]))
		end-- ;
	*end = '\0';
	cmdlen = end-cmd ;
	char *cmdparts[10] = {0};
	unsigned num_parts = 0 ;
	char *save_ptr ;
	char *tok = strtok_r(cmd,"\r\n \t",&save_ptr);
	while (tok && (num_parts < ARRAYSIZE(cmdparts))) {
		cmdparts[num_parts++] = tok ;
		tok = strtok_r(0,"\r\n \t",&save_ptr);
	}
#ifdef DEBUG_MSGS
	for( unsigned i = 0 ; i < num_parts ; i++ )
		printf( "%d: <%s>\n", i, cmdparts[i]);
#endif

	if ((0 == strcasecmp(cmdparts[0],"send")) && (3 == num_parts)) {
		unsigned len ;
		unescape(cmdparts[2],len);
		if (sldev.send((stronglink_t::command_e)strtoul(cmdparts[1],0,0),
			       cmdparts[2], len,0)) {
			printf( "sent %u bytes\n", len);
		}
		else
			printf ("error sending data\n");
	} else if (0 == strcasecmp(cmdparts[0],"serial")) {
		long long serial ;
		if (sldev.serialNumber(serial)) {
			printf ("serial #%llx\n", serial);
		} else {
			printf ("no serial number\n");
		}
	} else {
		fprintf(stderr, "Unknown command %s\n", cmdparts[0]);
	}
}

int main (int argc, char const * const argv[])
{
	if (2 > argc) {
		fprintf(stderr, "Usage: %s deviceName\n", argv[0]);
		return -1 ;
	}
	stronglink_t sl(argv[1]);
	int cardstate = DONTKNOW ;
	unsigned iteration = 0 ;
	if (sl.isOpen()) {
		printf ("opened %s\n", argv[1]);
		prompt(cardstate);
		while (1) {
			int numReady ;
			struct pollfd fds[2];
			fds[0].fd = sl.getFd();
			fds[0].events = POLLIN ;
			fds[1].fd = fileno(stdin);
			fds[1].events = POLLIN|POLLHUP ;
			if (0 < (numReady = poll(fds,ARRAYSIZE(fds),100))) {
				if (fds[0].revents) {
					enum stronglink_t::response_e resp ;
					void const *rxdata ;
					unsigned rxlen ;
					void *context ;
					if (sl.receive(resp,rxdata,rxlen,context)) {
						printf ("response: %02x ", resp);
						char const *bytes = (char const *)rxdata ;
						for (unsigned i = 0 ; i < rxlen ; i++) {
							printf( "%02x ", *bytes++ );
						}
						printf("\n");
						prompt(cardstate);
					}
				}
				if (fds[1].revents & POLLIN) {
					char inbuf[512];
					int numRead = read(fileno(stdin),inbuf,sizeof(inbuf));
					if ((1 < numRead) && (CARDPRESENT == cardstate)){
						process_command(sl,inbuf,numRead);
					} else if (0 == numRead) {
						break;
					}
					prompt(cardstate);
				}
				else if (fds[1].revents & POLLHUP) {
					break;
				}
			} else {
				if (sl.send(sl.SL_SELECTCARD,0,0,(void*)iteration)) {
					while (0 < (numReady = poll(fds,1,1000))) {
						enum stronglink_t::response_e resp ;
						void const *rxdata ;
						unsigned rxlen ;
						void *context ;
						if (sl.receive(resp,rxdata,rxlen,context)) {
							if (stronglink_t::SLSTAT_SUCCESS == resp) {
								if (CARDPRESENT != cardstate) {
									char const *bytes = (char const *)rxdata ;
									if (1 <= rxlen) {
										unsigned type = bytes[rxlen-1];
										printf("card type %d/%s\n",type,
										       type < ARRAYSIZE(cardTypeNames)
										       ? cardTypeNames[type] : "unknown");
										long long serial ;
										if (sl.serialNumber(serial)) {
											printf ("serial #%llx\n", serial);
										} else {
											printf ("no serial number\n");
										}
									}
#ifdef DEBUG_MSGS
									printf ("card present: datalen %d\n", rxlen);
									for (unsigned i = 0 ; i < rxlen ; i++) {
										printf ("%02x ", *bytes++ );
									}
									printf("\n");
#endif
									cardstate = CARDPRESENT ;
									prompt(cardstate);
								}
							} else if (stronglink_t::SLSTAT_NOCARD == resp) {
								if (CARDNOTPRESENT != cardstate) {
									cardstate = CARDNOTPRESENT ;
									prompt(cardstate);
								}
							} else {
								fprintf(stderr, "Unknown response %d to poll\n", resp);
								prompt(cardstate);
							}
							break;
						}
					}
				} else {
					fprintf(stderr, "poll send error\n");
					return -1 ;
				}
			}
		}
	}

	return 0 ;
}
#endif
