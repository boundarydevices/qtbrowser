/*
 * Module name: print.cpp
 *
 * Copyright 2011, Boundary Devices
 */
#include "print.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <QProcess>

printer_t::printer_t() {
}

printer_t::~printer_t() {
}

bool printer_t::print(QPixmap const &pic) {
	printf("%s:%ux%u, colorspace %u\n", __PRETTY_FUNCTION__,pic.width(), pic.height(), pic.depth() );
	char const *printcmd = getenv("PRINTCMD");
	if (printcmd) {
		QProcess proc ;
		proc.start(printcmd);
		if (proc.waitForStarted()) {
			printf( "opened %s\n", printcmd);
			if (pic.save(&proc,"PNG")) {
				proc.closeWriteChannel();
				return proc.waitForFinished();
			}
			else
				printf( "%s: error saving img\n", __PRETTY_FUNCTION__ );
		}
		else
			printf( "%s: error starting %s\n", __PRETTY_FUNCTION__, printcmd );
	} else
		fprintf (stderr, "%s: missing PRINTCMD\n", __PRETTY_FUNCTION__ );

	return false ;
}
