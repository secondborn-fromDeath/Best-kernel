
#include <generic/types.h>
#include <conf/processing.cfg>
#include <other/userspace_environment.h>



Class Thread{
	Process * parent;
	State * state;			//this includes, at the start, rip,cs,rsp,ss,rflags from the tss and then also all general purpose and vectors
	tid_t tid;
	sig_t sigmask;
	sig_t sigset;
	(void (*)(void * data))[MAXSIG];
	ustd_t stacksize;
};

Class Process{
//	pid_t pid;
	user_t owner_id;
	sig_t sigmask;
	sig_t sigset;
	void * pagetree;
	tid_t wk_cnt;
	Thread ** workers;
	descriptor_t max_desc;	//cant change at process-runtime
	ustd_t descriptors;	//current
	Descriptor ** descs;
};

Class Processor{
	privilege_t privilege;	//needed for faults
	ulong_t current_thread;
	Thread ** queue;
	ustd_t queued_threads;
};


enum signals{
	SIGKILL;
	SIGTERM;
	SIGSTOP;
	SIGCONT;
	SIGOFLOW;
	SIGUFLOW;
	SIGFPE;
	SIGPOLLING;
	SIGIO;
	SIGUSR0;
	SIGUSR1;
	SIGUSR2;
	SIGUSR3;
	SIGUSR4;
	SIGUSR5;
}
#define MAXSIG SIGUSR5;






