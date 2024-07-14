void * fmap(ustd_t findex, ustd_t amount, ustd_t offset){	//offset is absolute and you are guaranteed that at least the smallpage under it is safe, amount is rounded up to the file's pagetype, determined at mount-time
	Kingmem * mm; get_kingmem_object(mm);
	Virtual_fs * vfs; get_vfs_object(vfs);
	ustd_t processor_id = stream_init(kings.MEMORY);
	Thread * calling_thread; get_thread_object(calling_thread);
	Process * calling_process = calling_thread->parent;
	void * backup = mm->vm_ram_table;
	mm->vm_ram_table = calling_process->pagetree;

	File * file = &vfs->descriptions[findex];
	ustd_t filemulti = get_multi_from_pagetype(file->mapped_pagetype);
	ustd_t passlen = amoutn/filemulti;
	if (amount%filemulti){++passlen;}		//NOTE hardening on amount==0
	void * ret = mm->mem_map(file->shared_contents[offset/filemulti],file->mapped_pagetype,passlen,cache.WRITEBACK) + offset%filemulti;

	mm->vm_ram_table = backup;
	__nontemporal mm->calendar[processor_id] = 0;

	return ret;
}
