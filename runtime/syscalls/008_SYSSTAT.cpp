/*
This fills a userspace structure with information about a file in the same format as the File.meta structure*/
void SYS_STAT(ustd_t desc, meta * returns){
	Thread * thread = get_thread_object(void);
	Process * process = thread->parent;
	Kingmem * mm = get_kingmem_object(void);

	CONDITIONAL_SYSRET(thread,(!(process->descs->count > desc)||(process->descs->ckarray[desc] == 0)),1);

	auto * true_ret = mm->vmto_phys(process->pagetree,returns);
	CONDITIONAL_SYSRET(thread,true_ret == 0,2);

	memcpy(true_ret,&get_vfs_object(void)->descriptions[process->descs->pool[descs]->findex]->meta,sizeof(meta));
	thread->sys_retval = 0;
	SYSRET;
}