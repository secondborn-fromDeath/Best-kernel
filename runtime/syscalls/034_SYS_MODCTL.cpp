//this is going to call a function from the specified module
ustd_t module_ctl(fiDriv * driv, ustd_t function, void * data){		//now this is kind of awkward, matter of fact, did i free that from the driver's address sapce when doing this?
	Thread * thread = get_thread_object(NUH);
	Thread * templ = driv->d->runtime->children->pool[0];

	Thread * callee = clone_thread(templ);

	callee->state.stack_pointer -= 16;
	ulong_t * something = vmto_phys(callee->father->pagetree,callee->state.stack_pointer)[0];
	something[0] = NULLPTR;	if (data){
		something[0] = mm->mem_map(driv->driver->runtime->pagetree,data,pag.SMALLPAGE,1,cache.WRITEBACK);
	}
	something[1] = NULLPTR;
	if !(something[0]){ return something[1];}

	if (function > callee->parent->code->methodsnum){ return 1;}	//bail D:

	driv->d->stream_init(NUH);
	callee->state.instruction_pointer = callee->parent->userspace_code + callee->parent->code->functions[function];
	run_thread(callee);
}


void SYS_MODCTL(ustd_t descriptor, ustd_t function, void * data){
	Thread * calling_thread = get_thread_object(NUH);
	Process * process = get_process_object(NUH);

	CONDITIONAL_SYSRET(thread,process->owner_id != owner_ids::ROOT,1);
	CONDITIONAL_SYSRET(thread,((descriptor > process->descs->length)||(process->ckarray[descriptor] == 0)),2);

	fiDriv * driv = &vfs->descriptions[process->descs->pool[descriptor]->findex];

	Kingmem * mm = get_kingmem_object(NUH);
	auto * passdata = mm->vmto_phys(driv->runtime->pagetree,data);
	CONDITIONAL_SYSRET(thread,passdata == 0,3);

	thread->sys_retval = module_ctl(driv,function,passdata);
	SYSRET;
}
