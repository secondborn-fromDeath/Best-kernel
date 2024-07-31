/*
This sets the threads target microseconds to be ARGUMENT higher than the current microseconds time*/
void SYS_SLEEP(ulong_t sleep){
	Thread * thread = get_thread_object(void);
	thread->target_micros = gettime_nanos(void)*1000 + sleep;
	thread->sys_retval = 0;
	SYSRET;
}
