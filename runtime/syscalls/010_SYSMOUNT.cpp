void SYSMOUNT(char * partition){
	Virtual_fs * vfs = get_vfs_object(void);
	Thread * thread = get_thread_object(void);
	Kingmem * mm = get_kingmem_object(void);

	auto * truepart = mm->vmto_phys(process->pagetree,part);
	ustd_t len;
	ustd_t findex;
	vfs->open(truepart,0,&len,&findex);

	thread->sys_retval = vfs->mount(findex,partition);
	SYSRET;
}
