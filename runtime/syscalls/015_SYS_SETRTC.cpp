/*
This sets the time using a struct rtcret, see time.h
*/
void SYS_SETTIME(struct rtc_time * new){
	Thread * thread = get_thread_object(void);
	Process * process = thread->parent;
	Kingmem * mm = get_kingmem_object(void);

	CONDITIONAL_SYSRET(thread,process->owner_id != owner_ids::ROOT,1);

	auto * pointer = mm->vmto_phys(process->pagetree,new);
	CONDITIONAL_SYSRET(thread,pointer == 0,2);

	set_timespec(pointer);
	thread->sys_retval = 0;
	SYSRET;
}
