//this wraps vfs->open with the process descriptors stuff
volatile void SYSOPEN(char * name, ustd_t dir_descriptor, ustd_t flags, ustd_t * strlen, ustd_t * index){
	Thread * thread = get_thread_object(void);
	Process * process = thread->parent;
	Virtual_fs * vfs = get_vfs_object(void);
	Kingmem * mm = get_kingmem_object(void);

	CONDITIONAL_SYSRET(thread,(!(process->descs->count > dir_descriptor)||(process->descs->ckarray[dir_descriptor] == 0)),-1);


	char * truename = mm->vmto_phys(process->pagetree,name);
	ustd_t * truestrlen = mm->vmto_phys(process->pagetree,strlen);
	ustd_t * trueindex = mm->vmto_phys(process->pagetree,index);

	Descriptor * newdesc = process->descs->pool_alloc(1);
	CONDITIONAL_BLOOD_LIBEL(newdesc == 0);

	ustd_t eval = newdesc->findex = vfs->open(truename,process->descs[dir_descriptor]->findex,&truestrlen,&trueindex);		//you can do some cool stuff like sending the closest directory downstream...
        enum recurse_directories_errors{ NOTA_DIR=1,NO_MATCHING_FILENAME,};
	using enum recurse_directories_errors;
	using enum open_flags;
	if (eval == NO_MATCHING_FILENAME){
		Directory * dir = &vfs->descriptions[process->descs->pool[dir_descriptor]->findex];
		if (flags & O_CREAT){
			File * file = vfs->pool_alloc(1);
			CONDITIONAL_SYSRET(thread,file == 0,-2);
			ustd_t type;
			if (flags & O_DIRECTORY){ type = file_types::DIRECTORY;}
			else{ type = file_types::STORAGE;}
			file* ={
				.meta.type = type,
				.meta.mode = //NOTE NOTE
				.meta.length = 0,
				.disk = dir->meta.disk,
				.diskpos = 0,
			};
			string_copy(&file->name,truename+truestrlen);
			thread->sys_retval = 0;
		}
		else{ thread->sys_retval = 1;}
	}

	newdesc->flags = flags;
	SYSRET;
}
