void * fmap(ustd_t findex, ustd_t amount, ustd_t offset){	//offset is absolute and you are guaranteed that at least the smallpage under it is safe, amount is rounded up to the file's pagetype, determined at mount-time
	Kingmem * mm = get_kingmem_object(void);
	Virtual_fs * vfs = get_vfs_object(void);
	Thread * calling_thread = get_thread_object(void);
	Process * calling_process = calling_thread->parent;

	File * file = &vfs->descriptions[findex];
	ustd_t filemulti = mm->get_multi_from_pagetype(file->mapped_pagetype);
	ustd_t passlen = amount;
	tosmallpage(passlen);
	void * ret = mm->mem_map(process->pagetree,file->shared_contents[offset/filemulti],file->mapped_pagetype,passlen,cache.WRITEBACK) + offset%filemulti;

	return ret;
}
