//amoutn and offset are absolute because the mapped_pagetype for the file is transparent to the process
void * fmap(ustd_t findex, ustd_t amount, ustd_t offset){
	Kingmem * mm = get_kingmem_object(void);
	Virtual_fs * vfs = get_vfs_object(void);
	Thread * calling_thread = get_thread_object(void);
	Process * calling_process = calling_thread->parent;
	DisksKing * dking = get_disksking_object(void);

	File * file = &vfs->descriptions[findex];
	ustd_t filemulti = mm->get_multi_from_pagetype(file->shared_contents->mapped_pagetype);
	ustd_t g = amount/filemulti;
	ulong_t * entry = mm->vmtree_fetch(calling_process->pagetree,file->shared_contents->mapped_pagetype,g);
	entry = vmto_entry(calling_process->pagetree,entry);
	for (1; g; --g){	//NOTE DOUBLE FAULT HARDENING
		entry[g] = vfs->load_page(dking,file,(offset/filemulti)+g) | memreq_template(file->shared_contents->mapped_pagetype,mode.MAP,cache.WRITEBACK,0);;
		++file->shared_contents->ckarray[(offset/multi)+g];
	}

	return ret+offset;
}
