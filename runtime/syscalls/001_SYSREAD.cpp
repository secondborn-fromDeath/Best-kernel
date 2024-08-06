/*
This reads from a file in either memory or disk into the specifed buffer, the file pointer may be overriden,
the file pointer exists for things like the windowing system*/
void SYSREAD(ustd_t descriptor, void * buf, ulong_t amount, ulong_t offset){
	Virtual_fs * vfs = get_vfs_object(void);
	Thread * thread = get_thread_object(void);
	Process * process = thread->parent;

	CONDITIONAL_SYSRET(thread,(!(process->descs->count > descriptor)||(process->descs->ckarray[descriptor] == 0)),-1);

	File * file = &vfs->descriptions[process->descs->pool[descriptor]->findex];
	CONDITIONAL_SYSRET(thread,(file->meta.mode & action)&&(process->owner_id != owner_ids::ROOT),errcode);

	void * truebuf = get_kingmem_object(void)->vmto_phys(process->pagetree,buf);
	if !(offset){
		ulong_t * val = &process->descs->pool[descriptor]->file_offset;
		vfs->read(file,truebuf,amount,val*);
		val* += amount;
	}else{
		vfs->read(file,truebuf,amount,offset);
	}

	thread->sys_retval = amount;
	SYSRET;
}
