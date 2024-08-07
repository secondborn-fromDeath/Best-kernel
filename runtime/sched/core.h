class Descriptor{
	ustd_t findex;
	ustd_t flags;
	uint64_t stream_offset;
};class Descriptor{
	ustd_t findex;
	ustd_t flags;
	uint64_t stream_offset;
	char * maps;
};
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

struct state{
	uint64_t accumulator;
	uint64_t counter;
	uint64_t rdx;
	uint64_t rbx;
	uint64_t stack_pointer;
	uint64_t string_dest;
	uint64_t string_source;
	uint64_t frame_pointer;
	uint64_t eight;
	uint64_t nine;
	uint64_t ten;
	uint64_t eleven;
	uint64_t twelve;
	uint64_t thirteen;
	uint64_t fourteen;
	uint64_t fifteen;
	uint64_t flags;
	uint64_t instruction_override[16];
	uint64_t instruction_pointer;
	uint64_t fs;
	uint64_t ds;
};
enum thread_types{ DRIVER,MODULE,THREAD,DEVICE,KERNEL,};	//all have different behaviour
class Thread : King{
	Process * parent;
	State state;			//this includes, at the start, rip,cs,rsp,ss,rflags from the tss and then also all general purpose and vectors
	ustd_t type;
	auto * double_link;		//ergo. getting teh device structure inside of the input system calls...
	sig_t sigmask;
	sig_t sigset;
	(void (*)(void * data))[MAXSIG];
//	ustd_t stacksize;		//process-based
	ustd_t taken;
	ulong_t sys_retval;		//we execute them instantly and then schedule...
	pollfd * poll;			//physical pointer to the userspace array of polling structures and their number
	ustd_t pollnum;
	Thread * prior;			//see ioctl
	ulong_t target_micros;		//see sys_sleep
	void * heap;

	void stream_init(void){
		mutex(&this->taken);
        }
};
class Process : King{
	Thread ** workers : King.pool;
	user_t owner_id;
	sig_t sigmask;
	sig_t sigset;
	void * pagetree;
	void * userspace_code;		//below but virtual
	void * code;
	process_descriptors descs;
	ustd_t polls_count;		//if this is above 20 you swap off the pagetree
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
	ushort_t reqaction;
	ushort_t retaction;
};

namespace signals{
enum{
	SIGKILL;
	SIGTERM;
	SIGSTOP;
	SIGCONT;
	SIGOFLOW;		//i am talking about INTO...
	SIGFPE;
	SIGSIMD;
	BREAKPOINT;
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

struct state{
	uint64_t accumulator;
	uint64_t counter;
	uint64_t rdx;
	uint64_t rbx;
	uint64_t stack_pointer;
	uint64_t string_dest;
	uint64_t string_source;
	uint64_t frame_pointer;
	uint64_t eight;
	uint64_t nine;
	uint64_t ten;
	uint64_t eleven;
	uint64_t twelve;
	uint64_t thirteen;
	uint64_t fourteen;
	uint64_t fifteen;
	uint64_t flags;
	uint64_t instruction_pointer;
	uint64_t fs;
	uint64_t ds;
};
enum thread_types{ DRIVER,MODULE,THREAD,DEVICE,KERNEL,};	//all have different behaviour
class Thread : King{
	Process * parent;
	State state;			//this includes, at the start, rip,cs,rsp,ss,rflags from the tss and then also all general purpose and vectors
	ustd_t type;
	auto * double_link;		//ergo. getting teh device structure inside of the input system calls...
	sig_t sigmask;
	sig_t sigset;
	(void (*)(void * data))[MAXSIG];
//	ustd_t stacksize;		//process-based
	ustd_t taken;
	ulong_t sys_retval;		//we execute them instantly and then schedule...
	pollfd * poll;			//physical pointer to the userspace array of polling structures and their number
	ustd_t pollnum;
	Thread * prior;			//see ioctl
	ulong_t target_micros;		//see sys_sleep
	void * heap;

	void stream_init(void){
		mutex(&this->taken);
        }
};
class Process : King{
	Thread ** workers : King.pool;
	user_t owner_id;
	sig_t sigmask;
	sig_t sigset;
	void * pagetree;
	void * userspace_code;		//below but virtual
	void * code;
	process_descriptors descs;
	ustd_t polls_count;		//if this is above 20 you swap off the pagetree
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
	ushort_t reqaction;
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
