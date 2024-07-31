/*
This returns a 64bit integer which is the time that passed since the last 24hour cycle with nanosecond granularity*/
void SYS_GETTIME(void){
	Thread * thread = get_thread_object(void);
	thread = gettime_nanos(void);
	run_thread(thread);			//assuming that you would like to know the time in nanoseconds because there is some need for precision so...
}
