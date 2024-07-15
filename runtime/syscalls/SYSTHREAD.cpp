//on the tin it says that you are cloning everything (at least until we have flags, TODO) from the calling thread...
void thread(){
	Kingthread * ktrd; get_kingthread_object(ktrd);
	ustd_t processor_id = ktrd->stream_init(void);
	ulong_t thread_index = ktrd->pool_alloc(1);
	__non_temporal ktrd->calendar[processor_id] = 0;

	Thread * calling_thread = get_thread_object(void);
	memcpy(ktrd->pool[thread_index], calling_thread, sizeof(Thread));

	Kingptr * kptr; get_pointer_object(kptr);
	kptr->stream_init(void);
	ktrd->pool[thread_index].desc_indexes = kptr->pool_alloc(calling_thread->desc_count);
	kptr->pool[processor_id] = 0;
	__non_temporal kptr->calendar[processor_id] = 0;
	memcpy(ktrd->pool[thread_index].desc_indexes,calling_thread->desc_indexes,calling_thread->desc_count);

	//making a stack for the new thread
	Process * process = calling_thread->parent;
	void * treebackup = mm->vm_ram_table;
	mm->vm_ram_table = process->pagetree;
	void * entry = mm->vmtree_fetch(header->stacksize+1,pag.SMALLPAGE);
	ktrd->pool[thread_index]->state.stack_ptr = entry;
	entry = mm->vmto_entry(entry,&pagetype) + (header->stacksize+1)*8;
	for (ustd_t h = 0; h < header->stacksize; ++h){				//NOTE HARDENING
	    void * hold = mm->getphys_identity(1,pag.SMALLPAGE);
	    entry* = mm->memreq_template(pag.SMALLPAGE,MAP,WRITEBACK) | hold<<12;
	    entry -= 8;
	}
	entry* = 0;
}
