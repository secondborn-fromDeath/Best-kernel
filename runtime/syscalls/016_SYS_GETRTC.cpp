/*
Returns a structure rtc_time*/
void SYS_GETRTC(struct rtc_time rtcret){
	Thread * thread = get_thread_object(void);
	Process * process = thread->parent;
	Kingmem * mm = get_kingmem_object(void);

	auto * pointer = mm->vmto_phys(process->pagetree,rtcret);
	CONDITIONAL_SYSRET(thread,pointer == 0,1);

	set_timespec(pointer);
	thread->sys_retval = 0;
	SYSRET;
}
