/*
This gets called every 5 seconds or so from the scheduler code*/
void reenact_io(void){
	Virtual_fs * vfs = get_vfs_object(NUH);
	Directory * dev = &vfs->descriptions[1];
	Module_removal_stack * modrm_sp = get_modrm_stack_object(NUH);

	for (ustd_t i = 0; i < dev->children->length; ++i){				//taking all device mutexes
		if !(dev->children->ckarray[i]){
			continue;}
		dev->children->pool[i]->stream_init(NUH);
	}
	brothers_sleep(NUH);								//high priority, halting all brothers
	for (ustd_t i = 0; i < modrm_sp->length; ++i){					//removing all of the scheduler drivers
		if !(modrm_sp->ckarray[i]){
			continue;}
		modrm(modrm_sp->pool[i]);
	}
	Kingmem * mm = get_kingmem_object(NUH);						//using mm->gdt->pool because identity mapping
	Kingprocess * kprc = get_kingprocess_object(NUH);				//extracting pointers to the interrupt handler segments from every single process
	for (ustd_t i = 0; i < kprc->length; ++i){
		if !(kprc->ckarray[i]){
			continue;}
		Process * process = kprc->pool[i];
		for (ustd_t k = 0; k < 256; ++k){					//cpl bits are irrelevant
			void * start;
			ulong_t length;
			extract_segment_statistics(mm->gdt->pool,&start,&length);
			mm->mem_unmap(process->pagetree,start,tosmallpage(length),pag::SMALLPAGE);
		}
	}
	load_all_devices(NUH);								//enumerating all buses recursively again
	assign_vectors(NUH);								//whitelisting interrupt lines
	for (ustd_t i = 0; i < kprc->length; ++i){					//remapping into all of the processes
		if !(kprc->ckarray[i]){
			continue;}
		Process * process = kprc->pool[i];
		for (ustd_t k = 0; k < 256; ++k){
			void * start;
			ulong_t length;
			extract_segment_statistics(mm->gdt->pool,&start,&length);
			mm->mem_map(process->pagetree,start,tosmallpage(length),pag::SMALLPAGE,cache::WRITEBACK);
		}
	}
	brothers_wake(NUH);
}
