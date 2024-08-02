/*
This removes the specified file from the virtual filesystem, if the file came from a disk it is deleted on disk also...*/
void SYS_RMFILE(ustd_t desc){
	Thread * thread = get_thread_object(void);
	Process * process = thread->parent;
	Virtual_fs * vfs = get_vfs_object(void);
	Disksking * dking = get_disksking_object(void);

	CONDITIONAL_SYSRET(thread,(!(process->descs->length > desc)||(process->descs->ckarray[desc] == 0)),1);

	File * file = &vfs->descriptions[process->descs->pool[desc]->findex];
	CONDITIONAL_SYSRET(thread,((file->meta.mode & permissions::WRITE)&&(process->owner_id != owner_ids::ROOT)),2);

	dking->rmfile(file);
	file->parent->children->pool_free(file->double_link,1);
	vfs->pool_free(file);

	thread->sys_retval = 0;
	SYSRET;
}
