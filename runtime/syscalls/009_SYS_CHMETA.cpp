/*
This changes a file's metadata to reflect the arguments userspace has passed
the name argument must be smaller than one page*/
void SYS_CHMETA(ustd_t desc, char * name, ustd_t type, ustd_t mode){
	Thread * thread = get_thread_object(void);
	Process * process = thread->parent;
	Virtual_fs * vfs = get_vfs_object(void);
	Kingmem * mm = get_kingmem_object(void);

	CONDITIONAL_SYSRET(thread,(!(process->descs->count > desc)||(process->descs->ckarray[desc] == 0)),1);

	File * file = &vfs->descriptions[process->descs->pool[desc]->findex];
	CONDITIONAL_SYSRET(thread,((file->meta.mode & permissions::WRITE)&&(process->owner_id != owner_ids::ROOT)),2);

	auto * truename = mm->vmto_phys(process->pagetree,name);
	memcpy(&file->meta.name,truename,sizeof(File.meta.name));
	file->meta.type = type;
	file->meta.mode = mode;

	thread->sys_retval = 0;
	SYSRET;
}
