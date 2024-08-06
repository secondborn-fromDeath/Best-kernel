//amount and offset are absolute because the mapped_pagetype for the file is transparent to the process
void * fmap(Process * process, File * file, ustd_t desc, ustd_t amount, ustd_t offset){
	Kingmem * mm = get_kingmem_object(NUH);

	ustd_t pages_number = pagetype_roundup(file->mem->mapped_pagetype,amount);
	ustd_t multi = get_multi_from_pagetype(file->mem->mapped_pagetype);

	void * vm = mm->vmtree_fetch(process->pagetree,file->mem->mapped_pagetype,pages_number);
	if !(vm){ return vm;}
	ulong_t * entry = vmto_entry(calling_process->pagetree,vm);
	ustd_t t = offset/multi + pages_number;
	ustd_t i = 0;
	for (ustd_t g = offset/multi; g < t; ++g){	//NOTE DOUBLE FAULT HARDENING
		void * pipe = desc<<13 | g<<46;

		entry[i] = pipe | memreq_template(file->shared_contents->mapped_pagetype,mode.MAP,cache.WRITEBACK,0);
		++file->shared_contents->ckarray[g];
		++i;
	}
	return vm;
}

void SYS_FMAP(ustd_t desc, ustd_t amount, ustd_t offset){
	Thread * thread = get_thread_object(NUH);
	Process * process = thread->parent;
	CONDITIONAL_SYSRET(thread,desc > 1<<14,-3);										//see the top of mem/core.h
	CONDITIONAL_SYSRET(thread,(!(process->descs->count > desc)||(process->descs->ckarray[desc] == 0)),-1);

	Virtual_fs * vfs = get_vfs_object(NUH);

	File * file = &vfs->descriptions[process->descs->pool[desc]->findex];
	CONDITIONAL_SYSRET(thread,((file->meta.mode & permissions::WRITE)&&(process->owner_id != owner_ids::ROOT)),-2);

	char ** maps = &process->descs->pool[desc]->maps;
	if (maps* == 0){
		maps* = kptralloc(512/8);
	}
	ustd_t pureoff = offset/get_multi_from_pagetype(file->mem->mapped_pagetype);
	for (ustd_t g = 0; g < pagetype_roundup(amount,file->mem->mapped_pagetype); ++g){
		if (maps*[g+pureoff] == 1){
			process->sigset |= signals::SIGKILL;
		}
		maps*[g+pureoff] = 1;
	}

	thread->sys_retval = fmap(process,file,amount,offset);
	SYSRET;
}
