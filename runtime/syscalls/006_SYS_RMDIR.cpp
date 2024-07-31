/*
This fails if a directory is not empty*/
void SYS_RMDIR(ustd_t desc){
	Thread * thread = get_thread_object(void);
	Process * process = thread->parent;
	Virtual_fs * vfs = get_vfs_object(void);
	Disksking * dking = get_disksking_object(void);

	CONDITIONAL_SYSRET(thread,(!(process->descs->length > desc)||(process->descs->ckarray[desc] == 0)),1);

	Directory * dir = &vfs->descriptions[process->descs->pool[desc]->findex];
	CONDITIONAL_SYSRET(thread,(dir->mode<<6>>7) > process->owner_id,2);            //permissions

	CONDITIONAL_SYSRET(thread,dir->children->count,3);

	dking->rmdir(dir);
	dir->parent->children->pool_free(dir->double_link,1);
	vfs->pool_free(dir);

	thread->sys_retval = 0;
	SYSRET;
}
