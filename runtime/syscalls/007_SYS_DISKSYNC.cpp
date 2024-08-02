/*
This synchronizes the file on disk with the contents of the file in memory*/
void SYS_DISKSYNC(ustd_t desc){
	Thread * thread = get_thread_object(void);
	Process * process = thread->parent;
	Virtual_fs * vfs = get_vfs_object(void);
	Disksking * dking = get_disksking_object(void);

	CONDITIONAL_SYSRET(thread,(!(process->descs->count > desc)||(process->descs->ckarray[desc] == 0)),-1);

	File * file = &vfs->descriptions[process->descs->pool[desc]->findex];
	CONDITIONAL_SYSRET(thread,((file->meta.mode & permissions::WRITE)&&(process->owner_id != owner_ids::ROOT)),-2);

	dking->disk_sync(file);

	thread->sys_retval = 0;
	SYSRET;
}
