/*
The format (BEFF, the Better Executable File Format) specifies a header, an identification string, a checksum and a binary, the header is the following:
*/
class Exec_header{
	uchar_t heapsize;	//in 4096 byte pages
	uchar_t tls_size;
	uchar_t stacksize;
	ushort_t max_descriptors;
	ulong_t expected_memory_usage;	//this is in gigabytes
	ustd_t start;
	ustd_t programtype;
	uchar_t id[32];
	ulong_t checksum;
};

//The checksum is valid if subtracting it to the sum of the other fields ends up being 0
#define EXEC_CHECK_CHECKSUM{	\
if !(header->heapsize+header->tls_size+ header->stacksize+header->max_descriptors+header->start+header->expected_memory_usage+header->program_type+	\
((ustd_t *)header->id)[0]+((ustd_t *)header->id)[8]+((ustd_t *)header->id)[16]+((ustd_t *)header->id)[24] == header->checksum)	\
}


/*this does not replace the calling process like oonix and instead simply makes a new one
	Reading the header into a page, parsing it (evaluating the checksum really)
	reserving the pages for the executable, making the process structure and initializing it,
	allocating stuff into the process' pagetree, setting the main thread's ip and sp
	mapping gdt and ldt into pagetree for tls and heap segments


NOTE DANGER MOST IMPORTANT PLEASE READ IT FUCK!
						remember that this heavily affects boot, setup_drivers()...
*/
//returns the index into the processes array
ulong_t exec(ustd_t findex){
	Kingmem * mm = get_kingmem_object(void);
	Virtual_fs * vfs = get_vfs_object(void);
	DisksKing * dking = get_disksking_object(void);


	Storage * source = &vfs->descriptions[findex];

	Exec_header * header = vfs->load_page(dking,source,0);

	EXEC_CHECK_CHECKSUM{
		//NOTE HARDENING CLEANUP ROUTINE, or not because we are going to swap the vfs anyway
		return -1;
	}

	Thread * calling_thread = get_thread_object(void);
	Process * calling_process = calling_thread->parent;
	if (header->program_type == programtypes.SYSTEM) && (calling_process->owner_id != users.ROOT){ return -2;}	//this is nonsense lol

	//loading the rest of the file
	ustd_t multi = mm->get_multi_from_pagetype(source->mapped_pagetype);
	for (ustd_t i = 1; i < vfs->descriptions[findex].meta.length; ++i){
		vfs->load_page(dking,source,i);
	}


	Kingprocess * kprc = get_kingprocess_object(void);
	Process * process = kprc->pool_alloc(1);

	process* = {
		.owner_id = calling_process->owner_id,		//inherited from the launching shell
		.sigmask = 0,
		.sigset = calling_process->sigset,		//preventing weird hackkxxs
		.workers->count = 1,
		.descs->maximum = header->max_descriptors,
		.calendar = 0;
		.code = header;
	};


	Kingptr * ptr = get_pointer_object(void);
	process->workers->pool = kptr->pool_alloc(1);
	process->descs->pool = kptr->pool_alloc(2);

	Kingthread * ktrd = get_kingthread_object(void);
	Thread * main_thread = ktrd->pool_alloc(1);		//NOTE RACE CONDITION ->taken needs to be within alloc
	main_thread->taken = 1;
	process->workers->pool[0] = main_thread;

	Kingdescs * kdescs = get_kingdescriptors_object(void);
	process->descs->pool[0] = kdescs->pool_alloc(1);
	process->descs->pool[1] = kdescs->pool_alloc(1);
	process->descs->pool[0]->index = calling_process->descs->pool[0]->index;	//same root context as the parent
	process->descs->pool[1]->index = DEVDESC;				//this is just /dev
	kdescs->calendary = 0;

	//NOTE mke it a function
	process->pagetree_length = 512*8;				//finding the length of the pagetree
	ustd_t mul = 512;
	for (ustd_t i = header->expected_memory_usage; i; --i){
		process->pagetree_length += mul*expected_memory_usage;
		mul *= 512;
	}

	//making the tree and piping it into Kingmem
	process->pagetree = malloc(tosmallpage(process->pagetree_length),pag.SMALLPAGE);
	mm->vmtree_lay(process->pagetree,header->expected_memory_usage);

	//NOTE you can simplify this iwth fmap() maybe?
	//mapping the executable file
	process->userspace_code = mm->mem_map(process->pagetree,source->shared_contents->pool[0],pagetype,1,cache.WRITEBACK);
	main_thread->state->instruction_pointer = header->start + entry;
	ustd_t pagetype;
	ulong_t * entry = mm->vmto_entry(process->pagetree,process->userspace_code,&pagetype);
	for (ustd_t g = 0; g < vfs->descriptions[findex].meta.length; ++g){
		entry[g] = mm->memreq_template(pagetype,mode.MAP,cache.WRITEBACK,1) | vfs->descriptions[findex]->shared_contents->pool[g]<<12;
		entry[g] |= 1<<63;		//setting pages as executable
	}


	//mapping the gdt
	process->gdt_linear = mm->mem_map(process->pagetree,mm->gdt->pool,pag.SMALLPAGE,tosmallpage(MAXA16BIT),cache.WRITEBACK);
	entry = mm->vmto_entry(process->pagetree,process->gdt_linear,pagetype);
	for (ustd_t g = 0; g < vfs->descriptions[findex].meta.length; ++g){
		entry[g] = mm->memreq_template(pag.SMALLPAGE,mode.MAP,cache.WRITEBACK,1) | mm->gdt->pool+4096*g;
		entry[g] ^= 1<<1;		//setting the pages as unreadable/writeable
	}
	//mapping the ldt
	process->local_descriptor_table->segment_selector = (mm->gdt->alloc(1)-mm->gdt->pool)/16;
	ustd_t ldtlen = tosmallpage((kontrol->max_threads+4+1)*16);
	void * actual_ldt = malloc(ldtlen,pag.SMALLPAGE);
	gdt_insert(64bit_sysseg_types::LDT,actual_ldt,pag.SMALLPAGE),ldt_entry);							//4*GP# + heap	NOTE boot hardening
	mm->mem_map(process->pagetree,actual_ldt,pag.SMALLPAGE,ldtlen,cache.WRITEBACK);

	process->local_descriptor_table ={
		.pool = actual_ldt;
		.length = ldtlen;
		.segment_selector = idt_index;
		.ckarray = malloc(1,pag.SMALLPAGE);
	};
	memset(process->local_descriptor_table->ckarray,1,6);		//firsy 4 + heap and mainthread tls

	//making a stack for the main thread
	make_stack(process->pagetree,main_thread);

	main_thread->state->fs = 4;		//heap segment
	process->local_descriptor_table->pool[4] = data_segment_insert(header->heapsize,pag.SMALLPAGE);	//NOTE HARDENING
	main_thread->state->gd = 5;		//thread local storage segment
	process->local_descriptor_table->pool[5] = data_segment_insert(header->tls_size,pag.SMALLPAGE);

	main_thread->type = thread_types::APPLICATION;	//type for syscalls
	main_thread->taken = 0;				//now runnable
	return process_index;
}
