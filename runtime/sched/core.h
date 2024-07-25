#include <generic/types.h>
#include <conf/processing.cfg>
#include <other/userspace_environment.h>



Class Thread{
	Process * parent;
	State * state;			//this includes, at the start, rip,cs,rsp,ss,rflags from the tss and then also all general purpose and vectors
	ustd_t type;			//module, thread
	tid_t tid;
	sig_t sigmask;
	sig_t sigset;
	(void (*)(void * data))[MAXSIG];
	ustd_t stacksize;
	ustd_t taken;
	ulong_t syscall_returnval;	//we execute them instantly and then schedule...
	pollfd * poll;			//pointer to the array of polling structures and their number
	ustd_t pollnum;
};

Class Process{
//	pid_t pid;
	user_t owner_id;
	sig_t sigmask;
	sig_t sigset;
	void * pagetree;
	void * code;
	tid_t wk_cnt;
	Thread ** workers;
	descriptor_t max_desc;		//cant change at process-runtime
	ustd_t descriptors;		//current
	Descriptor ** descs;
	ustd_t polls_count;		//if this is above 20 you swap off the pagetree
	LDT local_descriptor_table;
	void * gdt_linear;
};

Class Processor{
	ulong_t current_thread;
	ustd_t executed_threads;	//used in rescue_brothers(), as basically "is online"
	ustd_t online_capable;
};

enum polling_actions{ READING,WRITING,MUTEX,};		//... , ... , waiting for no other processes to have the file open			may be ORd
struct pollfd{
	ustd_t findex;
	ushort_t action;
	ushort_t retaction;
};

namespace signals{
enum{
	SIGKILL;
	SIGTERM;
	SIGSTOP;
	SIGCONT;
	SIGOFLOW;
	SIGUFLOW;
	SIGFPE;
	SIGPOLLING;
	SIGUSR0;
	SIGUSR1;
	SIGUSR2;
	SIGUSR3;
	SIGUSR4;
	SIGUSR5;
};
#define MAXSIG SIGUSR5;
};





