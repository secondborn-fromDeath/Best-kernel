/*
This returns the amount of seconds that passed since the epoch (as always, see time.h)*/
void SYS_GETTIME_EPOCH(void){
	Thread * thread = get_thread_obejct(void);
	thread->sys_retval = gettime_epoch(void);
	SYRET;
}
