/*
in this file is a list of all the system calls

it is subject to regular expansion
*/


enum{
	SYS_OPEN,
	SYS_CLOSE,
	SYS_READ,
	SYS_WRITE,
	SYS_DEL,
	SYS_DISKSYNC,
	SYS_STAT,
	SYS_CHMETA,
	SYS_MOUNT,
	SYS_SOCKET,	//these are not going to be doable until i have a networking subsystem
	SYS_BIND,
	SYS_LISTEN,
	SYS_ACCEPT,
	SYS_POLL,	//
	SYS_CHROOT,		//deletes all the prior descriptors context
	SYS_CONNECT,
	SYS_GETTIME,
	SYS_GETTIME_EPOCH,	//as epoch we are using september 16th 2019
  SYS_SETTIME,
	SYS_GETRTC,
	SYS_SETRTC,
	SYS_SLEEP,
	SYS_MUTEX,		//takes all 8aligned: function pointer, argsnum and arguments on the stack
	SYS_MALLOC,
	SYS_FREE,
	SYS_FMAP,
	SYS_FUNMAP,
	SYS_THREAD,
	SYS_FORK,
	SYS_EXEC,
	SYS_EXIT,
	SYS_SIGNAL_THREAD,
	SYS_SIGNAL_PROCESS,
	SYS_PSIGMASK,		//obviously kill, stop, term and such are going to be ignored
	SYS_TSIGMASK,
	SYS_SYSLOG,

	SYS_IOCTL,
	SYS_MODINSERT,
	SYS_MODRM,
};
