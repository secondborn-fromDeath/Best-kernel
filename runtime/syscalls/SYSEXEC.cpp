/*
The format (BEFF, the Better Executable File Format) specifies a header, an identification string, a checksum and a binary, the header is the following:
*/
Class Exec_header{
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

	mm->stream_init(void);
	dking->stream_init(void);
	Exec_header * header = vfs->load_page(dking,source,0);

	EXEC_CHECK_CHECKSUM{
		//NOTE HARDENING CLEANUP ROUTINE
		return -1;
	}
	Thread * calling_thread = get_thread_object(void);
	Process * calling_process = calling_thread->parent;
	if (header->program_type == programtypes.SYSTEM) && (calling_process->owner_id != users.ROOT){ return -2;}	//this is nonsense lol

	source->shared_contents[0] = header;
	ustd_t multi = get_multi_from_pagetype(source->mapped_pagetype);
	for (ustd_t i = 1; i < vfs->descriptions[findex].meta.length; ++i){
		vfs->load_page(dking,source,i);
	}

	__non_temporal dking->calendar = 0;
	__non_temporal mm->calendar = 0;


	Kingprocess * kprc = get_kingprocess_object(void);
	stream_init(kings.PROCESS);
	Process * process = kprc->pool_alloc(1);
	__non_temporal kprc->calendar = 0;

	process* = {
//		.pid = process_index,
		.owner_id = calling_process->owner_id,		//inherited from the launching shell
		.sigmask = 0,
		.sigset = calling_process->sigset,		//preventing weird hackkxxs
		.wk_cnt = 1,
		.max_desc = header->max_descriptors,
	};

	Kingptr * ptr = get_pointer_object(void);
	ptr->stream_init(void);
	process->workers = kptr->pool_alloc(1);
	process->descs = kptr->pool_alloc(2);
	__non_temporal ptr->calendary = 0;

	Kingthread * ktrd = get_kingthread_object(void);
	ktrd->stream_init(void);
	Thread * main_thread = ktrd->pool_alloc(1);
	process->workers[0] = main_thread;
	__non_temporal ktrd->calendar = 0;

	Kingdescs * kdescs = get_kingdescriptors_object(void);
	kdescs->stream_init(void);
	process->descs[0] = kdescs->pool_alloc(1);
	process->descs[1] = kdescs->pool_alloc(1);
	process->descs[0]->index = calling_process->descs[0]->index;	//same root context as the parent
	process->descs[1]->index = DEVDESC;				//this is just /dev
	kdescs->calendary = 0;

	mm->stream_init(void);
	ustd_t level_length = header->expected_memory_usage * 8 * 512;	//find the length of the pagetree with the expected memory usage and make the tree
	ustd_t pagetree_length = 512*8;
	for (ustd_t u = 0; u < pag.SMALLPAGE; ++u){
		page_length += level_length;
		level_length *= 512;
	}

	//making the tree and piping it into Kingmem
	process->pagetree = malloc(tosmallpage(pagetree_length),pag.SMALLPAGE);
	mm->vmtree_lay(process->pagetree,header->expected_memory_usage);

	void * treebackup = mm->vm_ram_table;
	mm->vm_ram_table = process.pagetree;

	//mapping the executable file into the pagetree
	mm->vmtree_fetch(vfs->descriptions[findex].meta.length,source->mapped_pagetype);
	ulong_t * entry = mm->mem_map(source->shared_contents[0],pagetype,1,cache.WRITEBACK);
	ustd_t pagetype;
	entry = mm->vmto_entry(entry,&pagetype);
	for (ustd_t g = 1; g < vfs->descriptions[findex].meta.length; ++g){
		entry[g] = mm->memreq_template(pagetype,mode.MAP,cache.WRITEBACK,1);		//signing the pages as executable
	}


	//mapping the gdt and ldt into the process' address space as unreadable and unwriteable
	entry = mm->mem_map(mm->gdt->pool,pag.SMALLPAGE,tosmallpage(MAX16BIT),cache.WRITEBACK);	//idk
	entry = vmto_entry(entry,&pagetype);
	entry ^= 1<<1;

	ustd_t maxthreads = (get_kingthread_object(void)->length/get_kingprocess_object(void)->length)*16;
	auto * ldt_entry = mm->gdt->pool_alloc(1);
	gdt_insert(64bit_sysseg_types::LDT,malloc(tosmallpage(maxthreads*16+4+1),pag.SMALLPAGE),ldt_entry);	//4+heap	NOTE boot hardening
	entry = mm->mem_alloc(ldt_entry,pag.SMALLPAGE,tosmallpage(MAX16BIT),cache.writeback);
	entry = vmto_entry(entry,&pagetype);
	entry ^= 1<<1;

	ustd_t ldt_index = (ldt_entry-mm->gdt->pool)/sizeof(mm->gdt->pool*);	//storing index into gdt
	process->local_descriptor_table ={
		.pool = ldt_entry;
		.length = tosmallpage(maxthreads*16+4+1);
		.gdt_index = idt_index;
		.ckarray = malloc(2,pag.SMALLPAGE);
	};
	memset(process->local_descriptor_table->ckarray,1,6);		//the first 4 cant be used because segment selectors have the funi gdt vs ldt bit

	//making a stack for the main thread
	make_stack(mm,main_thread);

	main_thread->state->fs = 4;		//heap segment
	process-local_descriptor_table->pool[4] = data_segment_insert(header->heapsize,pag.SMALLPAGE);	//NOTE HARDENING
	main_thread->state->gd = 5;		//thread local storage segment
	process-local_descriptor_table->pool[5] = data_segment_insert(header->tls_size,pag.SMALLPAGE);


	mm->vm_ram_table = treebackup;
	__non_temporal mm->calendar = 0;

	main_thread->type = thread_types::APPLICATION;
	return process_index;
}
