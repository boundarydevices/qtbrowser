#ifndef __STRONGLINK_H__
#define __STRONGLINK_H__

/*
 * This module defines an interface to a Stronglink SL-032 serial
 * RFID (smart card) reader/writer over a serial interface.
 *
 * It's primary purpose is setup, framing, and checksum calculation. 
 * Most message content is left to the application.
 *
 * In order to use this, an application will call the send() method with
 * the command and data, then call receive() routine, presumably in response
 * to a poll() readiness indicator until it returns true or until a timeout 
 * occurs. 
 * 
 * The stronglink_t class also keeps track of the serial number of the card
 * while it's present
 */
class stronglink_t {
public:
	stronglink_t (char const *devname);
	~stronglink_t(void);

	enum command_e {
		SL_SELECTCARD        = 0x01
	,	SL_LOGINSECTOR       = 0x02
	,	SL_READDATABLOCK     = 0x03
	,	SL_WRITEDATABLOCK    = 0x04
	,	SL_READVALUEBLOCK    = 0x05
	,	SL_WRITEVALUEBLOCK   = 0x06
	,	SL_WRITEMASTERKEY    = 0x07
	,	SL_INCREMENTVALUE    = 0x08
	,	SL_DECREMENTVALUE    = 0x09
	,	SL_COPYVALUE         = 0x0A
	,	SL_READPAGE          = 0x10
	,	SL_WRITEPAGE         = 0x11
	,	SL_STOREKEY          = 0x12
	,	SL_LOGINSTORED       = 0x13
	,	SL_ANSWER_SELECT     = 0x20
	,	SL_EXCHANGE_TRANSP   = 0x21
	,	SL_SETPINS           = 0x40
	,	SL_POWERDOWN         = 0x50
	,	SL_RESET             = 0xFF
	};

	enum response_e {
		SLSTAT_SUCCESS       = 0x00
	,	SLSTAT_NOCARD        = 0x01
	,	SLSTAT_LOGINSUCCESS  = 0x02
	,	SLSTAT_LOGINFAIL     = 0x03
	,	SLSTAT_READFAIL      = 0x04
	,	SLSTAT_WRITEFAIL     = 0x05
	,	SLSTAT_NOREADAFTER   = 0x06
	,	SLSTAT_ADDRESS_OVFL  = 0x08
	,	SLSTAT_COLLISION     = 0x0A
	,	SLSTAT_NOAUTH        = 0x0D
	,	SLSTAT_NOTVALUE      = 0x0E
	,	SLSTAT_ATS_FAILED    = 0x10
	,	SLSTAT_CL_COMMERR    = 0x11
	,	SLSTAT_CHECKSUMERR   = 0xF0
	,	SLSTAT_CMDCODEERR    = 0xF1
	};

	enum cardType_e {
		SLNOCARD
	,	SL1K
	,	SLPRO
	,	SLULTRALIGHT
	,	SL4K
	,	SLPROX
	,	SLDESFIRE
	};

	bool isOpen (void) const { return 0 <= fd_ ; }
	int getFd (void) const { return fd_ ; }

	bool send (enum command_e, void const *data, unsigned len, void *context);
	bool receive (enum response_e &value, void const *&data, unsigned &len, void *&context);

	bool serialNumber(long long &serial) const { serial = serial_ ; return 0LL != serial ; }

private:
	int		fd_ ;
	char		buf_[2048];
	int		len_ ;
        enum command_e 	prev_command_ ;
	long long	serial_ ;
};

#endif
