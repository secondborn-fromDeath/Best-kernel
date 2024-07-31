/*
Returns the process id of the calling process*/
void SYS_GETPID(void){
	Thread * thread = get_thread_object(void);
	Process * process = thread->parent;

	thread->sys_retval = (process-get_kingprocess_object(void)->pool)/sizeof(Process);
	SYSRET;
}
