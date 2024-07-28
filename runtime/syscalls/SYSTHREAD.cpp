//this is complete retardation, we need to solve the actual problem we had that caused us to do this shit instead




//takes mm with the process' tree already set by the way
void make_stack(Process * process, Thread * thread){
	Exec_header * header = thread->father->code;			//the other problem is "does this work fine when you have more than one stack"
	Kingmem * mm = get_kingmem_object(void);

	void * entry = mm->vmtree_fetch(process->pagetree,header->stacksize+1,pag.SMALLPAGE);	//DANGER assuming you are left at the bottom
	entry = mm->vmto_entry(process->pagetree,entry) + (header->stacksize+1)*8;

	thread->state.stack_pointer = entryto_vm(process->pagetree,entry);
	for (ustd_t h = 0; h < header->stacksize; ++h){
		entry* = (malloc(1,pag.SMALLPAGE)<<12) | memreq_template(pag.SMALLPAGE,mode.RESERVE,cache.WRITEBACK,0);	//non executable stack
		entry -= 8;
	}
	entry* = 0;	//guard page
}

//on the tin it says that you are cloning everything (at least until we have flags, TODO) from the calling thread...
void thread(void){
	Kingthread * ktrd = get_kingthread_object(void);
	Thread * newfag = ktrd->pool_alloc(1);

	Thread * calling_thread = get_thread_object(void);
	memcpy(newfag,calling_thread,sizeof(Thread));
	newfag->taken = 0;					//newfriend is not being taken care of by anybody unlike oldfren

	Kingptr * kptr = get_pointer_object(void);
	newfag->desc_indexes = kptr->pool_alloc(calling_thread->desc_count);
	memcpy(newfag.desc_indexes,calling_thread->desc_indexes,calling_thread->desc_count);

	//making a stack for the new thread
	make_stack(calling_thread->father,newfag);

	//assigning the heap and thread local storage segments
	newfag->state.fs = 4;
	newfag->state.gs = process->local_descriptor_table->segment_alloc(1);
}




