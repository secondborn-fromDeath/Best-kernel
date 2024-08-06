/*
This unallocates memory that was mapped through SYS_FMAP*/
void SYS_FUNMAP(void * vm, ustd_t amount){
	Thread * thread = get_thread_object(NUH);
	Process * process = thread->parent;

	ustd_t pagetype;
	void * trash = vmto_entry(vm,&pagetype);
	ustd_t multi = get_multi_from_pagetype(pagetype);
	Descriptor * d = process->descs->pool[(trash[0]<<16>>48)];

	Virtual_fs * vfs = get_vfs_object(NUH);
	File * file = &vfs->descriptions[d->findex];
	void * phys = ((trash*)<<16>>29)/multi;
	ustd_t i;
	for (i = 0; i < file->mem->length; ++i){		//getting the starting page
		if !(phys < file->mem->pool[i]){ break}
	}
	CONDITIONAL_SYSRET(thread, i > file->mem->length, 1);

	ustd_t keyse = amount/multi;
	CONDNITIONAL_SYSRET(thread,keyse,2);
	for (ustd_t k = i; k < i+keyse; ++k){		//securing fake frees...
		if (maps[k] == 0){
			process->sigset |= signals::SIGKILL;
			SYSRET;
		}
	}
	for (ustd_t k = i; k < i+keyse; ++k){		//freeing stuff
		--file->mem->listeners[k];}

	thread->sys_retval = mem_unmap(process->pagetree,vm,amount/multi,pagetype);
	SYSRET;
}
