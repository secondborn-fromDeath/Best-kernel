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


#define CONDITIONAL_BLOOD_LIBEL(condition){ if (condition){ BLOOD_LIBEL(void);}

//returns the index into the processes array
ulong_t exec(File * source){
	Kingmem * mm = get_kingmem_object(void);
	Virtual_fs * vfs = get_vfs_object(void);
	DisksKing * dking = get_disksking_object(void);

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
	for (ustd_t i = 1; i < source->meta.length; ++i){
		 CONDITIONAL_BLOOD_LIBEL(vfs->load_page(dking,source,i) == 0);
	}


	Kingprocess * kprc = get_kingprocess_object(void);
	Process * process = kprc->pool_alloc(1);
	CONDITIONAL_BLOOD_LIBEL(process == 0);

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
	CONDITIONAL_BLOOD_LIBEL(process->workers->pool == 0);
	process->descs->pool = kptr->pool_alloc(2);
	CONDITINOAL_BLOOD_LIBEL(process->descs->pool == 0);

	Kingthread * ktrd = get_kingthread_object(void);
	Thread * main_thread = ktrd->pool_alloc(1);		//NOTE RACE CONDITION ->taken needs to be within alloc
	CONDITIONAL_BLOOD_LIBEL(main_thread == 0);
	main_thread->taken = 1;
	process->workers->pool[0] = main_thread;

	Kingdescs * kdescs = get_kingdescriptors_object(void);
	process->descs->pool[0] = kdescs->pool_alloc(1);
	process->descs->pool[1] = kdescs->pool_alloc(1);
	CONDITIONAL_BLOOD_LIBEL(process->descs->pool[1] == 0);
	process->descs->pool[0]->index = calling_process->descs->pool[0]->index;	//same root context as the parent
	process->descs->pool[1]->index = DEVDESC;					//this is just /dev
	kdescs->calendary = 0;


	//NOTE make it a function
	process->pagetree_length = 512*8;				//finding the length of the pagetree
	ustd_t mul = 512;
	for (ustd_t i = header->expected_memory_usage; i; --i){
		process->pagetree_length += mul*expected_memory_usage;
		mul *= 512;
	}

	//making the tree and piping it into Kingmem
	process->pagetree = malloc(tosmallpage(process->pagetree_length),pag.SMALLPAGE);
	CONDITIONAL_BLOOD_LIBEL(process->pagetree == 0);
	mm->vmtree_lay(process->pagetree,header->expected_memory_usage);
	ustd_t pagetype;	//whore...

	//identity mapping the gdt
	ulong_t * entry = mm->vmto_entry(process->pagetree,mm->gdt->pool,&pagetype);
	for (ustd_t g = 0; g < source->meta.length; ++g){
		entry[g] = mm->memreq_template(pag.SMALLPAGE,mode.MAP,cache.WRITEBACK,1) | mm->gdt->pool+4096*g;
		entry[g] ^= 1<<2;		//setting pages as supervisor only
	}
	//identity mapping all of the "shared" system segments (interrupts + tss)
	ulong_t gdt = mm->gdt->pool;
	for (ustd_t g = 0; g < GDT_LOW_ENTRIES; ++g){
		void * start;
		ulong_t length;
		extract_segment_statistics(&gdt[g],&start,&length);

		entry = mm->physto_entry(start);
		for (ustd_t y = 0; y < lenght/4096; ++y){
			entry[y] = ((start+i*4096)<<12) | mm->memreq_template(pag.SMALLPAGE,mode.MAP,cache.WRITEBACK,1) | 1<<2;	//executable because itetrrupts and supervisor obviously
		}
	}

	//-whatever- mapping the ldt
	process->local_descriptor_table->segment_selector = (mm->gdt->pool_alloc(1)-mm->gdt->pool)/16;		//NOTE i dont think there is a need to add checks here but know that if the divide by zero happens we are double faulting...
	ustd_t ldtlen = tosmallpage((kontrol->max_threads+4+1)*16);
	void * actual_ldt = malloc(ldtlen,pag.SMALLPAGE);
	CONDITIONAL_BLOOD_LIBEL(actual_ldt == 0);
	gdt_insert(64bit_sysseg_types::LDT,actual_ldt,pag.SMALLPAGE),ldt_entry);							//4*GP# + heap	NOTE boot hardening
	ulong_t something = mm->mem_map(process->pagetree,actual_ldt,pag.SMALLPAGE,ldtlen,cache.WRITEBACK);
	for (ustd_t o = 0; o < ldtlen; ++o){
		something[o] |= 1<<2;
	}


	//mapping the executable file
	process->userspace_code = mm->mem_map(process->pagetree,source->shared_contents->pool[0],pagetype,1,cache.WRITEBACK);
	main_thread->state->instruction_pointer = header->start + entry;
	ustd_t pagetype;
	ulong_t * entry = mm->vmto_entry(process->pagetree,process->userspace_code,&pagetype);
	for (ustd_t g = 0; g < source->meta.length; ++g){
		entry[g] = mm->memreq_template(pagetype,mode.MAP,cache.WRITEBACK,1) | source->shared_contents->pool[g]<<12;
		entry[g] |= 1<<63;		//setting pages as executable
	}


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

//returns the process id
void SYS_EXEC(ustd_t desc){
	Thread * thread = get_thread_object(void);
	Process * process = thread->father;
	CONDITIONAL_SYSRET(thread,((desc > process->descs->length)||(process->ckarray[descs] == 0)),1);

	Virtual_fs * vfs = get_vfs_object(void);
	File * file = &vfs->descriptions[process->descs->pool[desc]->findex];
	CONDITIONAL_SYSRET(thread,(!(file->meta.mode & (permissions::READ|permissions::WRITE))||(process->owner_id == owner_ids::ROOT),1);	//valid lisp lmao

	thread->sys_retval = exec(file);
	SYSRET;
}
