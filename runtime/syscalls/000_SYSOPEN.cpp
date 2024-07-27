//this wraps vfs->open with the process descriptors stuff
volatile void SYSOPEN(char * name, ustd_t dir_descriptor, ustd_t flags){
	Thread * thread = get_thread_object(void);
	Virtual_fs * vfs = get_vfs_object(void);
	Kingmem * mm = get_kingmem_object(void);
	char * truename = mm->vmto_phys(name);

	thread->syscall_retval = vfs->open(truename,process->descs[dir_descriptor]->findex,flags);
	sysret(void);
}
