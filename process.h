#ifndef __PROCESS_H__
#define __PROCESS_H__

/*
 * This header file declares the process_t class for use in
 * reading GPS sentences from an attached GPS receiver.
 */

#include <QApplication>
#include <QtGui>
#include <QMap>

class process_t ;

class childProcess_t : public QProcess {
	Q_OBJECT
public:
	childProcess_t ( process_t * parent );
	virtual 	~childProcess_t ();

	Q_PID	getpid(void) const { return saved_pid; }
private slots:
	void 	error (QProcess::ProcessError error);
	void 	finished (int exitCode, QProcess::ExitStatus exitStatus);
	void 	readyReadStandardError(void);
	void 	readyReadStandardOutput(void);
	void 	started (void);
	void 	stateChanged ( QProcess::ProcessState newState );
	void	bytesWritten( qint64 );
private:
	Q_PID	saved_pid ;
};

class process_t : public QObject {
	Q_OBJECT
public:
	process_t();
	~process_t();

	void shutdown(void);

	Q_INVOKABLE QString popen(QString);

	Q_INVOKABLE int start(QString);	/* returns pid */
	Q_INVOKABLE int start(QString, QStringList);	/* returns pid */
	Q_INVOKABLE bool stop(int pid);
	Q_INVOKABLE QString read(int pid, int fd);
	Q_INVOKABLE int write(int pid,QString s);
	Q_INVOKABLE int close_write(int pid);

	Q_INVOKABLE void exit(int retval);
	Q_INVOKABLE QStringList arguments(void);

signals:
	void onInput(int pid, int fd);
	void onExit(int pid, int exitstat);

private slots:
	void readData(int fd);

private:
	QMap<Q_PID,childProcess_t *>	processes ;
	friend class childProcess_t ;
	void started(childProcess_t *);
	void finished(childProcess_t *, int exitstat);
	void fdReady(childProcess_t *, int fd);
};

#endif
