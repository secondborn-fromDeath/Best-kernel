/*
This gives the calling thread a mask for signals*/
void SYS_TSIGMASK(ustd_t sigmask){
	Thread * thread = get_thread_object(void);

	CONDITIONAL_SYSRET(thread,sigmask & signals::SIGKILL,1);
	CONDITIONAL_SYSRET(thread,sigmask & signals::SIGTERM,2);
	CONDITIONAL_SYSRET(thread,sigmask & signals::SIGSTOP,3);

	thread->sigmask = sigmask;
	thread->sys_retval = 0;
	SYSRET;
}

