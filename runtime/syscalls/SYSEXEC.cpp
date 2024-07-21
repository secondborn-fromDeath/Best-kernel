/*
The format (BEFF, the Better Executable File Format) specifies a header, an identification string, a checksum and a binary, the header is the following:
*/
Class Exec_header{
	ushort_t heapsize;	//in 4096 byte pages
	uchart_t tls_size;
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
	stream_init(kings.POINTERS);
	process->workers = kptr->pool_alloc(1);
	process->descs = kptr->pool_alloc(2);
	__non_temporal ptr->calendary = 0;

	Kingthread * ktrd = get_kingthread_object(void);
	stream_init(kings.THREAD);
	process->workers[0] = ktrd->pool_alloc(1);
	__non_temporal ktrd->calendar = 0;

	Kingdescs * kdescs = get_kingdescriptors_object(void);
	stream_init(kings.DESCRIPTORS);
	process->descs[0] = kdescs->pool_alloc(1);
	process->descs[1] = kdescs->pool_alloc(1);
	process->descs[0]->index = calling_process->descs[0]->index;	//same root context as the parent
	process->descs[1]->index = DEVDESC;				//this is just /dev
	kdescs->calendary = 0;

	stream_init(kings.MEMORY);
	//find the length of the pagetree with the expected memory usage and make the tree
	ustd_t level_length = header->expected_memory_usage * 8 * 512;
	ustd_t pagetree_length = 512*8;
	for (ustd_t u = 0; u < pag.SMALLPAGE; ++u){
		page_length += level_length;
		level_length *= 512;
	}
	ustd_t aligned_pagetree_length;
	if (pagetree_lenght%4096){ aligned_pagetree_length = pagetree_length/4096+1;}	//losslessly rounding to pagesize
	else{ aligned_pagetree_length = pagetree_length/4096;}

	process->pagetree = malloc(aligned_pagetree_length,pag.SMALLPAGE);
	mm->vmtree_lay(process->pagetree,header->expected_memory_usage);

	//mapping the executable file into the pagetree
	mm->vmtree_fetch(vfs->descriptions[findex].meta.length,source->mapped_pagetype);
	ulong_t * entry = mm->mem_map(source->shared_contents[0],pagetype,1,cache.WRITEBACK);
	ustd_t pagetype;
	entry = mm->vmto_entry(entry,&pagetype);
	for (ustd_t g = 1; g < vfs->descriptions[findex].meta.length; ++g){
		entry[g] = mm->memreq_template(pagetype,mode.MAP,cache.WRITEBACK,1);		//signing the pages as executable
	}

	void * treebackup = mm->vm_ram_table;
	mm->vm_ram_table = process.pagetree;

	entry = mm->vmtree_fetch(header->stacksize+1,pag.SMALLPAGE);
	process->workers[0]->state.stack_ptr = stackp;
	entry = mm->vmto_entry(entry,&pagetype) + (header->stacksize+1)*8;
	for (ustd_t h = 0; h < header->stacksize; ++h){				//NOTE HARDENING
		void * hold = mm->getphys_identity(1,pag.SMALLPAGE);
		entry* = mm->memreq_template(pag.SMALLPAGE,mode.MAP,cache.WRITEBACK,0) | hold<<12;
		entry -= 8;
	}
	entry* = 0;	//guard page

	mm->vm_ram_table = treebackup;
	__non_temporal mm->calendar = 0;

	return process_index;
}
