/*
Signals a process, accounting for permissions*/
void SYS_PSIGNAL(ustd_t pid, ustd_t signal){
	Thread * thread = get_thread_object(void);
	Process * process = &get_kingprocess_object(void)->pool[pid];

	CONDITIONAL_SYSRET(thread,thread->parent->owner_id < process->owner_id,1);
	CONDITIONAL_SYSRET(thread,bit_pop_count(signal) != 1,2);

	process->sigset |= signal;
	SYSRET;
}
