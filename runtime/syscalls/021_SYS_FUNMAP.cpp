/*
This unallocates memory that was mapped through SYS_FMAP*/
void SYS_FUNMAP(void * vm, ustd_t amount){
	Thread * thread = get_thread_object(void);
	Process * process = thread->parent;

	ustd_t pagetype;
	void * trash = vmto_entry(vm,&pagetype);
	thread->sys_retval = mem_unmap(process->pagetree,vm,amount/get_multi_from_pagetype(pagetype),pagetype);
	SYSRET;
}
