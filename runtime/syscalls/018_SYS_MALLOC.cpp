/*
This allocated memory to the calling process*/
void SYS_MALLOC(ustd_t pages_number, ustd_t pagetype){		//smallpage is 3 and it descends as the pagesize gets larger
	Thread * thread = get_thread_object(void);
	CONDITIONAL_SYSRET(thread,((pagetype < 1)||(pagetype > 3)),1);
	CONDITIONAL_SYSRET(thread,pages_number > 512,1);	//DANGER these are unsigned but i dont like gcc fuckery (reminder x86 does ja and jb as the unsigned comparison instructions)

	Process * process = thread->parent;
	Kingmem * mm = get_kingmem-object(void)

	void * insertion = mem_alloc(process->pagetree,pagetype,pages_number);
	CONDITIONAL_SYSRET(thread,insertion == -1,2);

	process->used_memory += pages_number * get_multi_from_pagetype(pagetype) / get_multi_from_pagetype(pag.SMALLPAGE);
	thread->sys_retval = insertion;
	SYSRET;
}
