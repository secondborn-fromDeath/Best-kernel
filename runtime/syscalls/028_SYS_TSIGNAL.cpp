/*
Signals a thread of a process, accounts for permissions*/
void SYS_TSIGNAL(ustd_t tid, ustd_t signal){
	Kingthread * ktrd = get_kingsthread_object(void);
	Thread * calling_thread = get_thread_object(void);
	Thread * victim = &ktrd->pool[tid];

	CONDITIONAL_SYSRET(thread,victim->father->owner_id > calling_thread->father->owner_id,1);

	victim->sigset |= signal;
	calling_thread->sys_retval = 0;
	SYSRET;
}
