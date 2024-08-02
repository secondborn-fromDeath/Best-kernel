void SYSWRITE(ustd_t desc, void * buf, ustd_t amount, ustd_t offset){
	Virtual_fs * vfs = get_vfs_object(void);
	Thread * thread = get_thread_object(void);
	Process * process = thread->parent;

	CONDITIONAL_SYSRET(thread,(!(process->descs->count > desc)||(process->descs->ckarray[desc] == 0)),-1);

	File * file = &vfs->descriptions[process->descs->pool[desc]->findex];
	CONDITIONAL_SYSRET(thread,((file->meta.mode & permissions::WRITE)&&(process->owner_id != owner_ids::ROOT)));

	void * truebuf = vmto_phys(process->pagetree,buf);
	vfs->write(findex,truebuf,amount,offset);
	thread->sys_retval = amount;
	SYSRET;
}
