/*
This is a "do-nothing" ping for sockets*/
void SYS_PING(ustd_t descriptor){
	Thread * thread = get_thread_object(NUH);
	Process * process = thread->parent;

	CONDITIONAL_SYSRET(thread,!(descriptor < process->descs->length)||(process->ckarray[descriptor] == 0),1);

	report_IO(process->descs->pool[descriptor]->findex),pollable_actions::PING);

	thread->sys_retval = 0;
	SYSRET;
}
