/*
Returns the thread id of the calling thread*/
void SYS_GETTID(void){
	Thread * thread = get_thread_object(void);
	thread->sys_retval = (thread-get_threadking_objrct(void)->pool)/sizeof(Thread);
	SYSRET;
}
