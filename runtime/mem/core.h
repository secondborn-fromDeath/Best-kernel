/*
	They are using pointers meaning that you can use the first level as cushion without having to fix the next levels to fit it
	now this also means that you have the "opportunity" to do silly shit with the layout of the tables, i dont do that LOL
	each level is indeed forced to be 512*x


Functions:
all mutexes are called internally



		.getphys_free()				return array of physical pages
		.getphys_identity()			return first page of a contiguous streak
		.manipulate_phys()			takes either a SET or CLEAR argument, only needs the mutex on SET
		.vmtree_lay()				lay down a pagetree, returns the length in pages
		.vmtree_fetch()				get the first of a streak of entries in the tree
		.vmto_entry()				return pointer to corresponding entry
		.entryto_vm()				returns vm pointer, given the entry
		.vmto_phys()				return physical translation
		.deallocate_vm()			return physical translation and reset entries to default
		.memreq_template()			pipe page settings into a "template"

		kernel API:				see kernlib/mem.h
		.malloc()				reserved IDENTITY MAPPED memory
		.mrealloc()			above, copy on write if there is nomem
		.mfree()				macro for manipulate_mem

		usrspc API:
		.memalloc()				return pointer to virtually contiguous memory
		.memmap()				return pointer to physically contiguous specified memory, no care for reservation status
		.memfree()				deallocate_vm and frees entries in the physical table
		.memunmap()				deallocate_vm
		.memrealloc()				old_length dealloc, newlength alloc
	//	.memremap()				no use

		.softfault_handler()			called on soft faults
		.hardfault_handler()			above but on nonpresence

		.exchange_swap()			returns the address the pagetree entry was previously pointing to and inserts settings
		.swap_process()				goes through a process' pagetree and swaps entries out to disk

Bits:
		9					if set it means the page is taken, for directories refer to PS bit to see wether they can be traversed for the smaller pages within
		10-11					0 means segfault, 1 means map, 2 means reserve, 3 means swapped
		57-62					virtual disk number for swapped pages, reserved (ramdisk) in all other cases
		on file mapping 57-62 hold an index into the descriptors table for filemapped descriptors


Due to the need for the top bits we do 4level paging, nobody has that much memory anyway
*/

enum pag{ LEV4,BIGPAGE,MIDPAGE,SMALLPAGE};
class Kingmem{
	/*
	recurse directory, if you dont find what you need move on to the next
	the state you need for the downwards recursion:
		quota, stack of offsets, stack of tables(&), the success condition, fail on last offset of last table in highest level dir fails
	*/
	void * vmtree_fetch(void * pagetree, ustd_t pages_number, ustd_t pagetype){
		ulong_t * tables[pag.SMALLPAGE-pagetype+1];
		tables[0] = pagetree;
		void * offsets = NULL;
		ustd_t quota = 1;

		memset(&tables[1],0,4*(pag.SMALLPAGE-pagetype));
		memset(offsets,0,4*(pag.SMALLPAGE-pagetype));

		for (ustd_t su = 0; su < pagetype; ++su);
			ustd_t boo = 0;
			for (ustd_t h = 0; h < 512; ++h){
//				if (tables[su][h] & 1){ break;}		because i need to be able to do multiple stacks
				++boo;
				if ((tables[su][h] & 512)&&(pagetype == pag.SMALLPAGE))||((pagetype != pag.SMALLPAGE)&&(!(tables[su][h]] & 128))){//LOL
					 boo = 0;
				}
				if !(tables[su][h] & 1 ){ --boo;}	//again multiple stacks
				if (boo = quota){ break;}
			}
			if !(boo == quota){
				do {
					offsets += 1<<(12+(pag.SMALLPAGE-su)*9);
					if (offsets<<64-(64-(12+(pag.SMALLPAGE-su)*9))>>64-(12+(pag.SMALLPAGE-su)*9) == 512){ --su;}
					else {break;}
				}while (su);
				if !(su){ return 1;}
			}
			if (su+1 == pagetype){ quota = pages_number;}
		}
		//signing directories upwards
		for (ustd_t k = 0, k < pagetype; ++k){
			tables[k][offsets<<64-(64-(12+su*9))>>64-(12+(pag.SMALLPAGE-k)*9)] |= 1024;
		}
		return offsets;
	}
	void vmtree_lay(ulong_t * location, uchar_t expected_usage){	//in gigs
		void * default_entry = memreq_template(pag.BIGPAGE,mode.RESERVE,cache.WRITEBACK);

		ustd_t level_len = 512;
		ulong_t pointer = location+512*8;
		for (ustd_t level = 0; level < pag.SMALLPAGE; ++level){
			for (ustd_t h = 0; h < level_len; ++h){
				location[h] = default_entry | (pointer<<13);
			}
			level_len = expected_usage*8;
			pointer += level_len;
		}
		for (ustd_t h = 0; h < level_len; ++h){
			location[h] = default_entry | (pointer<<12);
		}
	}
	enum cache{ STRONGUNCACHEABLE,WRITETHROUGH,WRITEBACK,WEAKUNCACHEABLE,WRITEPROTECT,WRITECOMBINING,};	//needed for setting write_combining and write_protect through the mtrrs
	void * memreq_template(ustd_t pagetype, ustd_t mode, ustd_t cache, ustd_t exec){
		if (pagetype != pag.SMALLPAGE){
			ror32(cache,16);
			(ushort_t)cache <<=5;
			rol32(cache,16);
		}
		return (mode<<9) | cache | exec<<63;
	}
	ulong_t * vmto_entry(uint64_t * pagetree, void * vm, ustd_t * pagetype){
		pagetype* = 0;
		void * base = pagetree;
		ustd_t bit = pag.SMALLPAGE*9+12;
		for (ustd_t g = 0; g < pag.SMALLPAGE; ++g){
			ulong_t * readable = &base[((vm<<64)-(64-bit))>>(64-bit)];
			if (readable* & 1){  return NULL;}
			if (readable* & 512){ pagetype* = g; return readable;}
			base = ((readable*)<<(64-(12+36)))<<(12-64+(12+36)));	//DANGER signed compile time shit
		}
		ulong_t * readable = &base[(vm<<(64-(64-bit)))>>(64-bit)];		//reminder this is fine because syscall server is identity mapped
		if (readable* & 1){ return NULL;}
		return readable;
	}
	void * vmto_phys(void * pagetree, void * vm){
		ustd_t pagetype;
		void * entry = vmto_entry(pagetree, vm,&pagetype);
		if (pagetype == pag.SMALLPAGE){ return (entry<<(64-(12+36)))<<(12+(64-(12+36)));}
		return (entry<<(64-(12+36)))<<(13+(64-(13+36)));
	}
	void * entryto_vm(void * entry){
		Process * process = get_process_object(void);
		ustd_t whatever = entry - process->pagetree;
		void * vm = NULLPTR;
		for (ustd_t i = pag.SMALLPAGE; i; --i){
			ustd_t g = get_multi_from_pagetype(i);
			vm |= (whatever/g)<<(12+9*i);
			whatever %= g;
		}
		return vm;
	}
	ustd_t deallocate_vm(void * pagetree, void ** physrets, void * vm, ustd_t pages_number){
		ustd_t pagetype;
		ulong_t * entries = vmto_entry(pagetree,vm,&pagetype);
		if !(entries){ return 1;}
		ustd_t shift = 12;
		if (pagetype != pag.SMALLPAGE){ ++shift;}
		physret[0] = (entries* <<(64-(shift+36)))<<(shift+(64-(shift+36)))
		entries* = memreq_template(pagetype,mode.RESERVE,cache.WRITEBACK);
		for (ustd_t h = 1; h < pages_number; ++h){
			physret[h] = (entries[h] <<(64-(shift+36)))<<(shift+(64-(shift+36)))
			entries[h] = memreq_template(pagetype,mode.RESERVE,cache.WRITEBACK);
		}
		return 0;
	}
	//you are already blocking so you might as well not waste the bytes...
	ustd_t get_multi_from_pagetype(ustd_t pagetype){
		switch (pagetype){
			case pag.MIDPAGE:{ return 4096*512;}
			case pag.BIGPAGE:{ return 4096*512*512;}
			default:{ return 4096;}
		}
	}
	/*
	Array of bits where each bit stands for a 4096 byte page, we have alignment requirements for the allocation, returned 1 doesnt negate the contents
	of the returned array meaning that you can get whatever is available (interactive syscalls?)
	*/
	ustd_t get_free_phys(void ** physrets, ustd_t pages_number, ustd_t pagetype){
		this->stream_init(void);
		ustd_t multiplier = get_multi_from_pagetype(pagetype);
		ustd_t bit = 1;
		ustd_t gate = 0;
		ustd_t phys_cnt = 0;
		for (ustd_t indexer = 0; indexer < this->memlen/8; 0){
			if !(bit){ ++bit; ++indexer;}
			if (gate == multiplier){
				ustd_t off = 0;
				for (ustd_t g = bit; g; g*=2){ ++off;}
				physrets[phys_cnt] = (indexer+off)*multiplier;
				++phys_cnt;
			}
			++gate;
			if (this->phys_ram_table[indexer] & bit){
				gate = 0;
				if !((indexer*8)%multiplier){ indexer += multiplier/8;}
				else{ indexer = indexer/(multiplier/8) + multiplier/8;}
			}
			bit<<=2;
			if (phys_cnt == pages_number){
				__non_temporal this->calendar = 0;
				return 0;
			}
		}
		__non_temporal this->calendar = 0;
		return 1;
	}
	void * get_free_identity(ustd_t pages_number, ustd_t pagetype){
		this->stream_init(void);
		ustd_t multiplier = get_multi_from_pagetype(pagetype);
		ustd_t bit = 1;
		ustd_t gate = 0;
		for (ustd_t indexer = 0; indexer < this->memlen/8; 0){
			if !(bit){ ++bit; ++indexer;}
			if (gate == multiplier*pages_number){
				ustd_t off = 0;
				for (ustd_t g = bit; g; g*=2){ ++off;}
				__non_temporal this->calendar = 0;
				return (indexer+off)*multiplier;
			}
			++gate;
			if (this->phys_ram_table[indexer] & bit){
				gate = 0;
				if !((indexer*8)%multiplier){ indexer += multiplier/8;}
				else{ indexer = indexer/(multiplier/8) + multiplier/8;}
			}
			bit<<=2;
		}
		__non_temporal this->calendar = 0;
		return NULLPTR;
	}
	//this being one function is kind of silly...
	enum actions{ SET,CLEAR};
	void manipulate_phys(void ** phys, ustd_t pages_number, ustd_t pagetype, ustd_t action){
		if (action == actions.SET){ this->stream_init(void);}
		ustd_t quota = get_multi_from_pagetype(pagetype);
		for (ustd_t i = 0; i < pages_number; ++i){
			ustd_t indexer = phys/8/4096;
			ustd_t bit = 1;
			for (ustd_t j = 0; j < quota; ++j){
				this->phys_ram_table[indexer] |= bit;
				if (action == actions.CLEAR){ this->phys_ram_table[indexer] ^= bit;}
				bit *= 2;
				if !(bit){ ++bit;}
			}
		}
		if (action == actions.SET){ __non_temporal this->calendar = 0;}
	}
	void * mem_alloc(void * pagetree, ustd_t pagetype, ustd_t pages_number){
		void * templ = memreq_template(pagetype,mode.RESERVE,0);
		void * ret = vmtree_fetch(pagetree,pages_number,pagetype);
		ulong_t * off = vmto_entry(pagetree,ret,&pagetype);
		for (ustd_t h = 0; h < this->dir_entries[pagetype]; ++h){
			off[h] = templ;
		}
	}
	void * mem_map(void * pagetree, void * target, ustd_t pagetype, ustd_t pages_number, ustd_t cache){
		void * templ = memreq_template(pagetype,mode.MAP,cache);
		void * ret = vmtree_fetch(pagetree,pages_number,pagetype);
		ulong_t * off = vmto_entry(pagetree,ret,&pagetype);
		ustd_t addition = get_multi_from_pagetype(pagetype);
		ustd_t bit = 12; if (pagetype == pag.SMALLPAGE){ ++bit;}
		for (ustd_t h = 0; h < this->dir_entries[pagetype]; ++h){
			off[h] = templ | (target+addition*h)<<bit;
		}
	}
	void mem_free(void * pagetree, void * vm, ustd_t pages_number, ustd_t pagetype){
		void * phys[pages_number];
		deallocate_vm(phys,vm,pages_number);			//NOTE optimization
		manipulate_phys(phys,pages_number,pagetype,CLEAR);
		void * templ = memreq_template(pagetype,mode.RESERVE,cache.WRITEBACK);
		ulong_t off = vmto_entry(vm);
		for (ustd_t h = 0; h < this->dir_entries[pagetype]; ++h){
			off[h] = templ;
		}
	}
	void mem_unmap(void * pagetree, void * vm, ustd_t pages_number, ustd_t pagetype){
		void * templ = memreq_template(pagetype,mode.RESERVE,cache.WRITEBACK);
		ustd_t trash;
		ulong_t off = vmto_entry(pagetree,vm,&trash);
		for (ustd_t h = 0; h < this->dir_entries[pagetype]; ++h){
			off[h] = templ;
		}
	}
	//NOTE HARDENING this only supports extending the mapping
	void mem_realloc(void * pagetree, void * vm, ustd_t oldpages, ustd_t newpages, ustd_t pagetype){	//"Keep your pointer, son. I will take it from here."
		void * phys[pages_number];
		deallocate_vm(pagetree,phys,vm,oldpages);
		vmtree_fetch(pagetree,newpages,pagetype);
		void * templ = memreq_template(pagetype,mode.RESERVE,cache.WRITEBACK);
		ulong_t off = vmto_entry(pagetree,vm);
		ustd_t bit = 12; if (pagetype == pag.SMALLPAGE){ ++bit;}
		for (ustd_t h = 0; h < this->dir_entries[pagetype]; ++h){
			off[h] = templ | phys[h]<<bit;
		}
	}
	void mem_remap(void * pagetree, void * vm, ustd_t oldpages, ustd_t newpages, ustd_t pagetype){
		void * phys[pages_number];
		deallocate_vm(pagetree,phys,vm,oldpages);
		vmtree_fetch(pagetree,newpages,pagetype);
		void * templ = memreq_template(pagetype,mode.RESERVE,cache.WRITEBACK);
		ulong_t off = vmto_entry(pagetree,vm);
		ustd_t addition = get_multi_from_pagetype(pagetype);
		ustd_t bit = 12; if (pagetype == pag.SMALLPAGE){ ++bit;}
		for (ustd_t h = 0; h < this->dir_entries[pagetype]; ++h){
			off[h] = templ | (phys+addition*h)<<bit;
		}
	}
	void * exchange_swap(ulong_t * entry, void * diskpos){
		ulong_t new_entry = (entry*)<<55>>55;		//preserving the bottom 9 bits
		void * ret = (entry*)<<5>>18<<1;
		entry* = diskpos | new_entry | 3<<10;		//SWAPPED setting
		return ret;
	}
	/*
	This swaps out to disks whatever pages have been marked as reserved in a process' pagetree
	*/
	void swap_process(void){
		DisksKing * dking = get_disksking_object(void);
		Process * process = get_process_object(void);
		Virtual_fs * vfs = get_vfs_object(void);
		Kingmem * mm = get_kingmem_object(void);

		process->mutex(void);

		//so basically i am resetting the entries after finding the position on disk each time, now you have to track how deep the i inside of the pagetree though
		ustd_t pagetype = 0;
		ustd_t scaler = 512*8;
		for (ustd_t i = 0; i < process->pagetree_length/8; ++i){
			if (mm->sizeof_memory/mm->used_memory > 2){ break;}	//swapping until half of memory is safe :D
			void * swappages_pos = 0;
			if ((process->pagetree[i] & 512)&&(pagetype == pag.SMALLPAGE))||((pagetype != pag.SMALLPAGE)&&(!(process->pagetree[i] & 128))){		//so awful
				if ((pagetype != pag.SMALLPAGE)&&(process->pagetree)){ break;}
				for(ustd_t g = 0; g < dking->length; ++g){
					ustd_t pages = mm->get_multi_from_pagetype(pagetype)/4096;
					swappages_pos = dking-swaps[g]->shm->pool_alloc(pages);
					if (swappages_pos){
						void * identity = exchange_swap(&process->pagetree[i],swappages_pos);
						dking->write(g,swappages_pos,identity,1,pagetype);
						mm->used_memory -= pages;
					break;}
				}
			}
			if !(swappages_pos){ BLOOD_LIBEL(void);}	//no more swap or mem, calling for a process sacrifice
			if (i == scaler){ ++pagetype; scaler *= 512;}
		}

		__non_temporal process->calendar = 0;
	}
	void fault_handler(void){
		enter_ringzero(void);
		Kingmem * mm = get_kingmem_object(void);
		void * vm = __asm__("mov %%cr2,%%rax\n\t");
		ustd_t pagetype;
		ulong_t * entry = mm->vmto_entry(vm,&pagetype);		//MASSIVE DANGER read the binary for how pagetype gets passed, if shit is grim use Kingmem (this is locked anyway)
		if (entry & 1){		//present bit, checking wether hard or soft fault
			mm->hardfault_handler(entry);
		}
		else{
			mm->softfault_handler(entry);
		}
	}
	void hardfault_handler(void * entry, ustd_t pagetype){
		Kingmem * mm = get_kingmem_object(void);

		switch(entry* <<(64-12))>>53)
 			case mode.RESERVE:{
				void * phys = malloc(1,pagetype);
			break;}
			case mode.MAP:{
				Process * process = get_process_object(void);
				ustd_t mapdescindex = (entry*) <<1>>58;		//omitting exec bit
				ustd_t i;
				for (i = 0; i < process->descnum; ++i){	// NOTE OPTIMIZATION
					if (process->descs[i]->flags & 1){ --mapdescindex;}
					if !(mapdescindex){ break;}
				}
				Virtual_fs * vfs = get_vfs_object(void);
				File * file = &vfs->descriptions[process->descs[i]->findex];

				ustd_t filemulti = mm->get_multi_from_pagetype(file->mapped_pagetype);
				ustd_t basechar = (entry*)<<5>>18<<1 / filemulti;

				if !(file->listeners[basechar]){
					void * newphys = get_free_identity(1,file->mapped_pagetype);
					DisksKing * dking = get_disksking_object(void);
					dking->stream_init(void);
					dking->read(file->disk,file->diskpos + filemulti*basechar,newphys,1,file->mapped_pagetype);
					__non_temporal dking->calendar = 0;

					entry* = ((entry*)<<5>>18<<1) | newphys;		//assuming that the address never gets silly this is fine
				}
				++file->listeners[basechar]; //reminder this isnt done in fmap/remap
			break;}
			case mode.SEGFAULT:{	//something something zero trust
				Process * process; get_process_object(process);
				process->sigset |= SIGSEGV;
			break;}
		}
	}
	void hardfault_handler(void * entry, ustd_t pagetype){
		Process * process = get_process_object(void);
		Kingmem * mm = get_kingmem_object(void);
		if (pagetype != pag.SMALLPAGE){++bit;}
		if ((entry*<<(64-12))>>54 == mode.SWAPPED){
			ustd_t processor_id = mm->stream_init(void);
			void * phys = malloc(1,pagetype);
			void * vmtab_backup = mm->vm_ram_table;
			mm->vm_ram_table = process->pagetree;
			void * newentry = mm->vmtree_fetch(1,pagetype);
			newentry = mm->memreq_template(pagetype,RESERVE,WRITEBACK) | (phys<<5>>18<<1);	//userspace means writeback is fit.
			mm->vm_ram_table = vmtab_backup;
			__non_temporal mm->calendar[processor_id] = 0;
			ustd_t disk = (entry*) >>57;
			void * doffset = (entry*) <<5>>18<<1;
			DisksKing * dking; get_disksking_object(dking);
			dking->stream_init(void);
			dking->read(disk,doffset,phys,1,pagetype);
			__non_temporal dking->calendar = 0;
			ustd_t pages = get_multi_from_pagetype(pagetype) / 4096;
			Virtual_fs * vfs; get_vfs_object(void);
			vfs->descriptions[dking->disks[disk]]->shm->stream_init(void);
			vfs->descriptions[dking->disks[disk]]->shm->pool_free(pages);
			__non_temporal vfs->descriptions[dking->disks[disk]]->shm->calendar = 0;
		}
		else{process->sigset |= SIGSEGV;}
	}
};
