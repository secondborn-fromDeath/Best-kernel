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
	uchar_t id[32];
	ulong_t checksum;
};


/*this does not replace the calling process like oonix and instead simply makes a new one
	Reading the header into a page, parsing it (evaluating the checksum really)
	reserving the pages for the executable, making the process structure and initializing it,
	allocating stuff into the process' pagetree, setting the main thread's ip and sp

NOTE this assumes that you never do pageswapping (see entire executable being loaded), with competent users and developers this is fine but
if the game engine is shit (You?... I? We maybe. Are the incompetent developers!!! ???) and you cant fix it then you will need to come back here.
*/
ustd_t exec(ustd_t findex){
	Kingmem * mm; get_kingmem_object(mm);
	Virtual_fs * vfs; get_vfs_object(vfs);
	DisksKing * dking; get_disksking_object(dking);

	ustd_t processor_id = stream_init(kings.MEMORY);

	Exec_header * header = mm->reserve_phys_identity(1,pag.SMALLPAGE);
	dking->read(vfs->descriptions[findex]->disk,vfs->descriptions[findex]->diskpos,header,1,pag.SMALLPAGE);	//mapping only the header so the system doesnt crash opening something like a gigabyte large video file
	//The checksum is valid if subtracting it to the sum of the other fields ends up being 0
	if !(header->heapsize+header->tls_size+ header->stacksize+header->max_desxriptors+header->start+((ustd_t *)header->id)[0]+	\
	((ustd_t *)header->id)[8]+((ustd_t *)header->id)[16]+((ustd_t *)header->id)[24]	== header->checksum){ return 1;}

	void * executable[vfs->descriptions[findex].meta.length];
	executable[0] = header;
	for (ustd_t i = 1; i < vfs->descriptions[findex].meta.length; ++i){
		executable[i] = mm->reserve_phys_identity(mm,1,SMALLPAGE);
		dking->read(vfs->descriptions[findex]->disk,vfs->descriptions[findex]->diskpos+i*4096,header,1,pag.SMALLPAGE);
	}
	__nontemporal mm->calendar[processor_id] = 0;

	Kingprocess * kprc; get_kingprocess_object(kprc);
	stream_init(kings.PROCESS);
	ulong_t process_index = kprc->pool_alloc(1);
	__nontemporal kprc->calendar[processor_id] = 0;

	Thread * calling_thread = get_thread_object();
	Process * calling_process = calling_thread->parent;
	kprc->pool[process_index] = {
//		.pid = process_index,
		.owner_id = calling_process->owner_id,		//inherited from the launching shell
		.sigmask = 0,
		.sigset = calling_process->sigset,		//preventing weird hackkxxs
		.wk_cnt = 1,
		.max_desc = header->max_descriptors,
	};

	Kingptr * ptr; get_pointer_object(ptr);
	stream_init(kings.POINTERS);
	kprc->pool[process_index]->workers = kptr->pool_alloc(1);
	kprc->pool[process_index]->descs = kptr->pool_alloc(2);
	__nontemporal ptr->calendary[processor_id] = 0;

	Kingthread * ktrd; get_kingthread_object(ktrd);
	stream_init(kings.THREAD);
	kprc->pool[process_index]->workers[0] = ktrd->pool_alloc(1);
	__nontemporal ktrd->calendar[processor_id] = 0

	Kingdescs * kdescs; get_kingdescriptors_object(kdescs);
	stream_init(kings.DESCRIPTORS);
	kprc->pool[process_index]->descs[0] = kdescs->pool_alloc(1);
	kprc->pool[process_index]->descs[1] = kdescs->pool_alloc(1);
	kprc->pool[process_index]->descs[0]->index = calling_process->descs[0]->index;	//same root context as the parent
	kprc->pool[process_index]->descs[1]->index = DEVDESC;				//this is just /dev
	kdescs->calendary[processor_id] = 0;

	stream_init(kings.MEMORY);
	//find the length of the pagetree with the expected memory usage and make the tree
	ustd_t level_length = header->expected_memory_usage * 8 * 512;
	ustd_t pagetree_length = 512*8;
	for (ustd_t u = 0; u < pag.SMALLPAGE; ++u){
		page_length += level_length;
		level_length *= 512;
	}
	ustd_t aligned_pagetree_length;
	if (pagetree_lenght%4096){ aligned_pagetree_length = pagetre_length/4096+1;}	//losslessly rounding to pagesize
	else{ aligned_pagetree_length = pagetree_length/4096;}

	kprc->pool[process_index]->pagetree = mm->getphys_identity(aligned_pagetree_length,pag.SMALLPAGE);
	mm->vmtree_lay(kprc->pool[process_index]->pagetree,header->expected_memory_usage);

	//anwyay, writing into the pagetree the executable that was read into memory
	mm->vmtree_fetch(vfs->descriptions[findex].meta.length,pag.SMALLPAGE);
	ulong_t * entry = mm->mem_map(executable[0],pagetype,1,cache.WRITEBACK);
	ustd_t pagetype;
	entry = mm->vmto_entry(entry,&pagetype);
	for (ustd_t g = 1; g < vfs->descriptions[findex].meta.length; ++g){
		mm->mem_map(executable[g],pagetype,1,cache.WRITEBACK);
		entry[g] |= 1<<63;		//signing the pages as executable
	}

	void * treebackup = mm->vm_ram_table;
	mm->vm_ram_table = kprc->pool[process_index].pagetree;

	entry = mm->vmtree_fetch(header->stacksize+1,pag.SMALLPAGE);
	kprc->pool[process_index]->workers[0]->state.stack_ptr = stackp;
	entry = mm->vmto_entry(entry,&pagetype) + (header->stacksize+1)*8;
	for (ustd_t h = 0; h < header->stacksize; ++h){				//NOTE HARDENING
		void * hold = mm->getphys_identity(1,pag.SMALLPAGE);
		entry* = mm->memreq_template(pag.SMALLPAGE,MAP,WRITEBACK) | hold<<12;
		entry -= 8;
	}
	entry* = 0;

	__nontemporal mm->calendar[processor_id] = 0;

	mm->vm_ram_table = treebackup;
	return 0;
}
