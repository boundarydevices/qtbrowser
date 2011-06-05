/*
 * This module file defines the methods of the process_t class for use in
 * starting sub-processes and reading and writing to their stdin, stdout,
 * and stderr file descriptors.
 */

#include "process.h"
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <linux/input.h>
#include <dirent.h>
#include <sys/ioctl.h>
#include <termios.h>

childProcess_t::childProcess_t (process_t * parent)
	: QProcess(parent)
{
	printf("%s\n", __PRETTY_FUNCTION__ );
	QObject::connect(this, SIGNAL(error (QProcess::ProcessError)), this, SLOT(error (QProcess::ProcessError)));
	QObject::connect(this, SIGNAL(finished (int, QProcess::ExitStatus)), this, SLOT(finished (int, QProcess::ExitStatus)));
	QObject::connect(this, SIGNAL(readyReadStandardError(void)), this, SLOT(readyReadStandardError(void)));
	QObject::connect(this, SIGNAL(readyReadStandardOutput(void)), this, SLOT(readyReadStandardOutput(void)));
	QObject::connect(this, SIGNAL(started (void)), this, SLOT(started (void)));
	QObject::connect(this, SIGNAL(stateChanged(QProcess::ProcessState)), this, SLOT(stateChanged(QProcess::ProcessState)));
	QObject::connect(this, SIGNAL(bytesWritten(qint64)), this, SLOT(bytesWritten(qint64)));
}

childProcess_t::~childProcess_t ()
{
	printf("%s\n", __PRETTY_FUNCTION__ );
}

void childProcess_t::error (QProcess::ProcessError error)
{
	printf("%s: %d\n", __PRETTY_FUNCTION__, error );
}

void childProcess_t::finished (int exitCode, QProcess::ExitStatus exitStatus)
{
	printf("%s: %d.%d\n", __PRETTY_FUNCTION__, exitCode, exitStatus );
	((process_t *)parent())->finished(this,exitCode);
	delete this ;
}

void childProcess_t::readyReadStandardError()
{
	((process_t *)parent())->fdReady(this,fileno(stderr));
}

void childProcess_t::readyReadStandardOutput()
{
	((process_t *)parent())->fdReady(this,fileno(stdout));
}

void childProcess_t::started ()
{
	saved_pid = pid();
//	printf("%s: pid %lld\n", __PRETTY_FUNCTION__, saved_pid );
	((process_t *)parent())->started(this);
}

void childProcess_t::stateChanged ( QProcess::ProcessState newState )
{
//	printf("%s: %d\n", __PRETTY_FUNCTION__, newState );
}

void childProcess_t::bytesWritten( qint64 count)
{
//	printf( "%s: %lld\n", __PRETTY_FUNCTION__, count);
}

process_t::process_t()
{
}

process_t::~process_t()
{
}

void process_t::readData(int fd)
{
}

QString process_t::popen(QString cmdline)
{
	QByteArray s ;
	s.reserve(0x1000);
	QByteArray ascii = cmdline.toAscii();
	FILE *fproc = ::popen(ascii.constData(), "r");
	if (fproc) {
		printf( "opened %s\n", ascii.constData());
		char inbuf[2048];
		int numRead ;
		while (0 < (numRead=fread(inbuf,1,sizeof(inbuf)-1,fproc))) {
			s.append(inbuf,numRead);
		}
		fclose(fproc);
	} else
		fprintf(stderr, "%s:%m\n", ascii.constData());
	return s ;
}

int process_t::start(QString program, QStringList args)
{
	childProcess_t *child = new childProcess_t(this);
        child->start(program,args);
	if (child->waitForStarted()) {
		processes[child->pid()] = child ;
		return child->pid();
	} else {
		fprintf(stderr, "Error %m starting %s\n", program.toAscii().constData());
	}
	return -1 ;
}

int process_t::start(QString program)
{
	childProcess_t *child = new childProcess_t(this);
        child->start(program);
	if (child->waitForStarted()) {
		processes[child->pid()] = child ;
		return child->pid();
	} else {
		fprintf(stderr, "Error %m starting %s\n", program.toAscii().constData());
	}
	return -1 ;
}

bool process_t::stop(int pid)
{
	printf("%s: %d\n", __PRETTY_FUNCTION__, pid);
	QMap<Q_PID,childProcess_t *>::const_iterator it = processes.find(pid);
	if (it != processes.end()) {
		(*it)->kill();
	} else {
		printf("%s: process %d not found\n", __PRETTY_FUNCTION__, pid);
	}
	return false ;
}

QString process_t::read(int pid, int fd)
{
	QMap<Q_PID,childProcess_t *>::const_iterator it = processes.find(pid);
	if (it != processes.end()) {
		if (1 == fd) {
			return (*it)->readAllStandardOutput();
		} else if (2 == fd) {
			return (*it)->readAllStandardError();
		}
	}
	return QString();
}

int process_t::write(int pid,QString s)
{
	printf("%s: write %s to pid %d\n", __PRETTY_FUNCTION__, s.toAscii().constData(), pid);
	int rval = -1 ;
	QMap<Q_PID,childProcess_t *>::const_iterator it = processes.find(pid);
	if (it != processes.end()) {
		rval = (*it)->write(s.toAscii());
		printf( "wrote %d, left %lld\n", rval, (*it)->bytesToWrite());
	}
	return rval;
}

Q_INVOKABLE int process_t::close_write(int pid)
{
	printf("%s: close write to pid %d\n", __PRETTY_FUNCTION__, pid);
	int rval = -1 ;
	QMap<Q_PID,childProcess_t *>::const_iterator it = processes.find(pid);
	if (it != processes.end()) {
		(*it)->closeWriteChannel();
		rval = 0 ;
	}
	return rval;
}

void process_t::started(childProcess_t *child)
{
	QMap<Q_PID,childProcess_t *>::const_iterator it = processes.find(child->getpid());
	if (it == processes.end()) {
		processes.insert(child->getpid(),child);
		printf ("%s: process %lld added to map\n", __PRETTY_FUNCTION__, child->getpid());
	} else {
		printf ("%s: duplicate process %lld\n", __PRETTY_FUNCTION__, child->getpid());
	}
}

void process_t::finished(childProcess_t *child,int exitStat)
{
	QMap<Q_PID,childProcess_t *>::iterator it = processes.find(child->getpid());
	if (it != processes.end()) {
		processes.erase(it);
		printf ("%s: process %lld finished\n", __PRETTY_FUNCTION__, child->getpid());
		emit onExit(child->getpid(), exitStat);
	} else {
		printf ("%s: process %lld not found\n", __PRETTY_FUNCTION__, child->getpid());
	}
}

void process_t::fdReady(childProcess_t *child, int fd) {
	emit onInput((int)child->getpid(), fd);
}

void process_t::exit(int retval) {
	QApplication::instance()->exit(retval);
}

QStringList process_t::arguments(void)
{
	return QApplication::arguments();
}
