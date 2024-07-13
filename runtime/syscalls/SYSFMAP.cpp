void * fmap(ustd_t findex, ustd_t pages_number, ustd_t pagetype, ustd_t offset){	//into the file, page aligned
	Kingmem * mm; get_kingmem_object(mm);
	Virtual_fs * vfs; get_vfs_object(vfs);
	ustd_t processor_id = stream_init(kings.MEMORY);
	readmiss(vfs,findex,mm,pages_number,pagetype);
	Thread * calling_thread; get_thread_object(calling_thread);
	Process * calling_process = calling_thread->parent;
	void * backup = mm->vm_ram_table;
	mm->vm_ram_table = calling_process->pagetree;
	ustd_t g = 4096; for(ustd_t i = pagetype, i < SMALLPAGE; ++i){ g*=512}
	void * ret = mem_map(mm,pages_number,pagetype,vfs->descriptions[findex].shared_contents+offset*g,0);	//identity mapped in syscall server btw
	mm->vm_ram_table = backup;
	__nontemporal mm->calendar[processor_id] = 0;

	return ret;
}
