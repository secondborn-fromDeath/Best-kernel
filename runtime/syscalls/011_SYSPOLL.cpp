/*
This is going to wait for input and output to signal to the thread that something happened
for now anythign happening is acceptable and userspace can decide what they want to do with the result of a call, see runtime/sched/core.h*/
void SYS_POLL(pollfd * structures){
	Thread * thread = get_thread_object(void);
	Process * process = thread->parent;
	Kingmem * mm = get_kingmem_obejct(void)

	thread->sigset |= SIGPOLLING;
	thread->poll = mm->vmto_phys(process->pagetree,structures);
	thread->sys_retval = 0;
	SYSRET;
}
