//this is going to call a function from the specified module
ustd_t module_ctl(ustd_t findex, ustd_t function, void * data){		//now this is kind of awkward, matter of fact, did i free that from the driver's address sapce when doing this?
	Virtual_fs * vfs = get_vfs_object(void);
	File * driv = &vfs->descriptions[findex];
	Thread * callee = driv->d->runtime->children->pool[0];

	Thread * thread = get_thread_object(void);
	CONDITIONAL_SYSRET(thread,thread->parent->owner_id < driv->owner_id,3);

	callee->stream_init(void);
	callee->prior = calling_thread;

	callee->state.stack_pointer -= 16;
	ulong_t * something = vmto_phys(callee->father->pagetree,callee->state.stack_pointer)[0];
	something[0] = mm->mem_map(driv->driver->runtime->pagetree,data,pag.SMALLPAGE,1,cache.WRITEBACK);
	something[1] = NULLPTR;
	if !(something[0]){ return something[1];}

	if (function > callee->parent->code->methodsnum){ return 1;}	//bail D:

	driv->d->stream_init(void);
	callee->state.instruction_pointer = callee->parent->userspace_code + callee->parent->code->functions[function];
	run_thread(callee);
}


void SYS_MODCTL(ustd_t descriptor, ustd_t function, void * data){
	Thread * calling_thread = get_thread_object(void);
	Process * process = get_process_object(void);
	CONDITIONAL_SYSRET(thread,((descriptor > process->descs->length)||(process->ckarray[descriptor] == 0)),1);

	ustd_t findex = process->descs->pool[descriptor]->findex;

	Kingmem * mm = get_kingmem_object(void);
	auto * passdata = mm->vmto_phys(driv->runtime->pagetree,data);

	CONDITIONAL_SYSRET(calling_thread,module_ctl(findex,function,passdata),2);	//see above
}
//SEE IOCTL FOR THE ctl_ret() function
