void SYSWRITE(ustd_t desc, void * buf, ustd_t amount, ustd_t offset){
	Virtual_fs * vfs = get_vfs_object(void);
	Thread * thread = get_thread_object(void);
	Process * process = thread->parent;

	CONDITIONAL_SYSRET(thread,(!(process->descs->count > desc)||(process->descs->ckarray[desc] == 0)),-1);

	void * pass = vmto_phys(process->pagetree,buf);
	vfs->write(process->descs->pool[desc]->findex,truebuf,amount,offset);		//we need to make teh above into a function and harden these check because shit is unacceptable
	thread->sys_retval = amount;
	SYSRET;
}
