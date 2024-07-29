#include <generic/types.h>
#include <conf/processing.cfg>
#include <other/userspace_environment.h>


class process_descriptors : Hash{
	Descriptor ** pool : Hash.pool;
	ustd_t maximum;
};
class process_threads : Hash{
	Thread ** pool : Hash.pool;
	ustd_t maximum;
};
class local_descriptor_table : King{
	uint64_t * pool : King.pool;
	ustd_t segment_selector;
};

enum thread_types{ DRIVER,MODULE,THREAD,DEVICE,};	//all have different behaviour
class Thread{
	Process * parent;
	State state;			//this includes, at the start, rip,cs,rsp,ss,rflags from the tss and then also all general purpose and vectors
	ustd_t type;
	auto * double_link;		//see type
	sig_t sigmask;
	sig_t sigset;
	(void (*)(void * data))[MAXSIG];
//	ustd_t stacksize;		//process-based
	ustd_t taken;
	ulong_t syscall_returnval;	//we execute them instantly and then schedule...
	pollfd * poll;			//physical pointer to the userspace array of polling structures and their number
	ustd_t pollnum;
	Thread * prior;			//see ioctl
};

class Process{
	user_t owner_id;
	sig_t sigmask;
	sig_t sigset;
	void * pagetree;
	void * userspace_code;		//below but virtual
	void * code;
	process_threads workers;
	process_descriptors descs;
	ustd_t polls_count;		//if this is above 20 you swap off the pagetree
	char calendar;			//used for EVERYTHING
	local_descriptor_table ldt;
	void * gdt_linear;
};

class Processor{
	ulong_t current_thread;
	ustd_t executed_threads;	//used in rescue_brothers(), as basically "is online"
	ustd_t online_capable;		//from acpi
};

enum polling_actions{ READING,WRITING,MUTEX,};		//... , ... , waiting for no other processes to have the file open			may be ORd
struct pollfd{
	ustd_t fdesc;
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
