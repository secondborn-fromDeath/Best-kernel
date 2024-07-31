void * make_stack(Process * process, Thread * thread){
	Exec_header * header = thread->father->code;			//the other problem is "does this work fine when you have more than one stack"
	Kingmem * mm = get_kingmem_object(void);

	void * ret = mm->vmtree_fetch(process->pagetree,header->stacksize+1,pag.SMALLPAGE);	//DANGER assuming you are left at the bottom
	if (ret == -1){ return NULLPTR;}
	ulong_t * entry = mm->vmto_entry(process->pagetree,entry) + (header->stacksize+1)*8;

	thread->state.stack_pointer = entryto_vm(process->pagetree,entry);
	for (ustd_t h = 0; h < header->stacksize; ++h){
		void * pipe = malloc(1,pag.SMALLPAGE);
		CONDITIONAL_BLOOD_LIBEL(pipe == 0);
		entry* = (pipe<<12) | memreq_template(pag.SMALLPAGE,mode.RESERVE,cache.WRITEBACK,0);	//non executable stack
		entry -= 8;
	}
	entry* = 0;	//guard page
	return ret;
}

//on the tin it says that you are cloning everything (at least until we have flags, TODO) from the calling thread...
void thread(void){
	Kingthread * ktrd = get_kingthread_object(void);
	Thread * newfag = ktrd->pool_alloc(1);
	CONDITIONAL_BLOOD_LIBEL(newfag == 0);

	memcpy(newfag,calling_thread,sizeof(Thread));

	//making a stack for the new thread
	void * term = make_stack(calling_thread->father,newfag);
	if !(term){ ktrd->pool_free(newfag,1); return 0;}
	newthread->state.stack_pointer = term;

	//assigning the heap and thread local storage segments
	newfag->state.fs = 4;
	newfag->state.gs = process->local_descriptor_table->pool_alloc(1);
	if (newfag->state.gs == 0){ calling_thread->parent->sigset |= SIGKILL;}		//meanies that make too many threads get the rope.

	newfag->taken = 0;
}

void SYS_THREAD(void){
	Thread * thread = get_thread_object(void);
	thread->sys_retval = thread(void);
	SYSRET;
}
