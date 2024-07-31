void SYSREAD(ustd_t descriptor, void * buf, ulong_t amount, ulong_t offset){
	Virtual_fs * vfs = get_vfs_object(void);
	Thread * thread = get_thread_object(void);
	Process * process = thread->parent;

	CONDITIONAL_SYSRET(thread,(!(process->descs->count > descriptor)||(process->descs->ckarray[descriptor] == 0)),-1);

	void * truebuf = get_kingmem_object(void)->vmto_phys(process->pagetree,buf);
	vfs->read(process->descs->pool[descriptor]->findex,truebuf,amount,offset);
	thread->sys_retval = amount;
	SYSRET;
}
