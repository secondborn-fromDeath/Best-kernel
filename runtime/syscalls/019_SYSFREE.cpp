/*
This frees memory that was reserved using the SYS_MALLOC function*/
void SYS_FREE(void * vm, ustd_t pages_number, ustd_t pagetype){
	Thread * thread = get_thread_object(void);
	CONDITIONAL_SYSRET(thread,((pagetype < 1)||(pagetype > 3)),1);
	CONDITIONAL_SYSRET(thread,pages_number > 512,1);

	Process * proces = thread->parent;
	mem_free(process->pagetree,vm,pages_number,pagetype);
	process->used_memory -= pages_number * get_multi_from_pagetype(pagetype) / get_multi_from_pagetype(pag.SMALLPAGE);

	thread->sys_retval = 0;
	SYSRET;
}
