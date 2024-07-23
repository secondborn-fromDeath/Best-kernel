//takes mm with the process' tree already set by the way
void make_stack(Kingmem * mm, Thread * thread, ){
	Exec_header * header = thread->father->code;

	void * entry = mm->vmtree_fetch(header->stacksize+1,pag.SMALLPAGE);
	thread->state.stack_ptr = (entry*)<<5>>18<<1;
	ustd_t pagetype;
	entry = mm->vmto_entry(entry,&pagetype) + (header->stacksize+1)*8;
	for (ustd_t h = 0; h < header->stacksize; ++h){				//NOTE HARDENING
	    void * hold = mm->getphys_identity(1,pag.SMALLPAGE);
	    entry* = mm->memreq_template(pag.SMALLPAGE,MAP,WRITEBACK) | hold<<12;
	    entry -= 8;
	}
	entry* = 0;	//guard page
}

//on the tin it says that you are cloning everyo
thing (at least until we have flags, TODO) from the calling thread...
void thread(void){
	Kingthread * ktrd = get_kingthread_object(void);
	ktrd->stream_init(void);
	Thread * newfag = ktrd->pool_alloc(1);
	__non_temporal ktrd->calendar = 0;

	Thread * calling_thread = get_thread_object(void);
	memcpy(newfag,calling_thread,sizeof(Thread));
	newfag->taken = 0;					//newfriend is not being taken care of by anybody unlike oldfren

	Kingptr * kptr = get_pointer_object(void);
	kptr->stream_init(void);
	newfag->desc_indexes = kptr->pool_alloc(calling_thread->desc_count);
	__non_temporal kptr->calendar = 0;
	memcpy(newfag.desc_indexes,calling_thread->desc_indexes,calling_thread->desc_count);

	//making a stack for the new thread
	mm->stream_init(void);
	void * treebackup = mm->vm_ram_table;
	mm->vm_ram_table = process->pagetree;
	make_stack(mm,newfag);
	mm->vm_ram_table = treebackup;
	__non_temporal mm->calendar = 0;

	//assigning the heap and thread local storage segments
	newfag->state.fs = 4;
	newfag->state.gs = process->local_descriptor_table->segment_alloc(1);
}











