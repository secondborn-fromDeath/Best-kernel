/*
You can do cool multiprocessing dependency-preventino stuff with it, really the main point is that the shell is going to sandbox all applications away from the user data
the path must be less than 4 kilobytes long.*/
void SYS_CHROOT(char * absolute_path){
	Thread * thread = get_thread_object(void);
	Process * process = thread->parent;
	Kingmem * mm = get_kingmem_object(void);

	char * true_path = mm->vmto_phys(process->pagetree,absolute_path);
	CONDITIONAL_SYSRET(thread,true_path == 0,1);

	for (ustd_t g = 0; 1; ++g){	//checking the length
		if (true_path[g] == 0){ break;}
		CONDITIONAL_SYSRET(thread,g == 4095,2);
	}

	Virtual_fs * vfs = get_vfs_object(void);
	ustd_t trash;
	ustd_t index;
	CONDITIONAL_SYSRET(thread,vfs->open(true_path,0,&trash,&index),3);

	File * newroot = &vfs->descriptions[index];
	thread->sys_retval = 0;
	SYSRET;
}
