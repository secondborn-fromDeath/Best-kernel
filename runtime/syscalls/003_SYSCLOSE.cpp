void SYSCLOSE(ustd_t desc){
	Thread * thread = get_thread_object(void);
	Process * process = thread->parent;

	CONDITIONAL_SYSRET(thread,(!(process->descs->count > desc)||(process->descs->ckarray[desc] == 0)),-1);

	process->descs->pool_free(&process->descs->pool[desc],1);
	--get_vfs_object(void)->descriptions[process->descs->pool[desc]->findex]->listeners;
	thread->sys_retval = 0;
	SYSRET;
}
