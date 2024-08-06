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
		56					if set it means the page is taken, for directories refer to PS bit to see wether they can be traversed for the smaller pages within
		9-11					0 means segfault, 1 means map, 2 means reserve, 3 means swapped, 4 means file
		57-62					virtual disk number for swapped pages, reserved (ramdisk) in all other cases
		on file mapping
					13-45 hold an index into the process' descriptors		//hard maximum of 4mill descriptors
					48-62 hold the idnex into the pages				//max... a lot of pages? too many to allocate in one go actually


Due to the need for the top bits we do 4level paging, nobody has that much memory anyway
*/

enum pag{ LEV4,BIGPAGE,MIDPAGE,SMALLPAGE};
class Kingmem{
	ulong_t offset_by_depth(Process * process, ustd_t pagetype){
		if !(pagetype){ return process->pagetree;}
		ulong_t ret = 512*8;
		ustd_t h = 512;
		for (ustd_t i = 1; i < pagetype; ++i){
			ret += h;
			h *= 512;
		}
		return ret;
	}
	void * vmtree_fetch(Process * process, ustd_t pages_number, ustd_t pagetype){
		process->stream_init(NUH);

		ustd_t iterator = 0;
		ustd_t endcond = gte_multi_from_pagetype(pagetype)/4096;
		uint64_t * pointer = offset_by_depth(process,pagetype);

		for (1; iterator < endcond; ++iterator){				//do we need argumented recursion or is this going to be fine? i think it is offsets fine.
			ustd_t j;
			for (j = 0; j < pages_number; +++j){
				if !(iterator%512){break;}				//bail on directory boundary
				if !(pointer[iterator] & 1){break;}			//bail if the entry is marked as unusable
				if (pointer[iterator] & 1<<56){break;}			//bail if an entry is occupied
			}
			if !(j == pages_number){ continue;}

			if (vmto_entry(process->pagetree,ret) == &pointer[iterator]){	//checking wether any fathering directories were signed, they are signed in vmto_entry...
				__non_temporal process->calendar = 0;
				return entryto_vm(process->pagetree,&pointer[iterator]);
			}
		}
		__non_temporal process->calendar = 0;
		return NULLPTR;
	}
	void vmtree_lay(ulong_t * location){	//in gigs
		void * default_entry = memreq_template(pag::BIGPAGE,mode::SEGFAULT,cache::WRITEBACK);
		ustd_t level_length = 512*sizeof(void *);

		uint64_t something = location+level_length;
		for (ustd_t i = 0; i < expected_usage; ++i){				//signing the toplevel we want to use
			location[i] = default_entry|((something+i*512)<<13);
		}
		for (ustd_t i = expected_usage; i < 512-expected_usage; ++i){		//signing useless pages as out-of-memory
			location[i] = default_entry|((something+i*512)<<13);
		}

		for (ustd_t g = 0; !(pag::SMALLPAGE<g); ++g){					//signing the rest of the pagetable
			location = something;
			level_length *= 512;
			for (ustd_t i = 0; i < level_length; ++i){
				location[i] = default_entry)
				if (g != pag::SMALLPAGE){
					location[i] |= (something+i*512)<<13;}
			}
		}
	}
	enum cache{ STRONGUNCACHEABLE,WRITETHROUGH,WRITEBACK,WEAKUNCACHEABLE,WRITEPROTECT,WRITECOMBINING,};
	void * memreq_template(ustd_t pagetype, ustd_t mode, ustd_t cache, ustd_t exec){
		if (pagetype != pag::SMALLPAGE){
			ror32(cache,16);
			(ushort_t)cache <<=5;
			rol32(cache,16);
		}
		return (mode<<9) | cache | exec<<63 | 1;
	}
	ulong_t * vmto_entry(uint64_t * pagetree, void * vm, ustd_t * pagetype){
		pagetype* = 0;
		ulong_t * ret = process->pagetree;
		for (ustd_t g = 1; g < pag::SMALLPAGE; ++g){
			ustd_t shift = 12+9*g - 9;
			ret += vm<<(64-shift)>>shift * get_multi_from_pagetype(pag::SMALLPAGE-g);
			ret* |= 1<<56;											//needed for vmtree_fetch, avoiding duplicate code
			if (ret* & 128){
				break;}
		}
		return ret;
	}
	void * vmto_phys(void * pagetree, void * vm){
		ustd_t pagetype;
		void * entry = vmto_entry(pagetree, vm,&pagetype);
		if !(entry){ return entry;}
		return (entry*)<<5>>18 + vm%(get_multi_from_pagetype(pagetype));
	}
	void * entryto_vm(void * entry){
		Process * process = get_process_object(NUH);
		ustd_t whatever = entry - process->pagetree;
		void * vm = NULLPTR;
		for (ustd_t i = pag::SMALLPAGE; i; --i){
			ustd_t g = get_multi_from_pagetype(i);
			vm |= (whatever/g)<<(12+9*i);
			whatever %= g;
		}
		return vm;
	}
	//useful for identity mapping stuff, returns the entry into the smallpages tables
	ulong_t * physto_offset(Process * process, void * phys){
		ulong_t offset = 512*8;
		for (ustd_t j = 0; j < pag::SMALLPAGE; ++j){
			offset += 512*offset;		//skipping the higher pagetables
		}
		ulong_t second = 0;								//now computing the offset using the pointer
		for (ustd_t y = 0; y < pag::SMALLPAGE; ++y){
			ustd_t g = this->get_multi_from_pagetype(y);
			second += phys/g(g*4096);
			phys %= g;
		}
		return process->pagetree+offset+second;
	}
	ustd_t deallocate_vm(Process * process, void ** physrets, void * vm, ustd_t pages_number){
		process->stream_init(NUH);
		ustd_t pagetype;
		ulong_t * entries = vmto_entry(process->pagetree,vm,&pagetype);
		if !(entries){ __non_temporal process->calendar = 0; return 1;}

		for (ustd_t h = 0; h < pages_number; ++h){		//looping twoce because it is less awkward then winding back, checking for the supervisor bit
			if (entries[g] & 4){ return 3;}}
		for (ustd_t h = 0; h < pages_number; ++h){
			physret[h] = entries[h]<<5>>18;
			entries[h] = memreq_template(pagetype,mode::SEGFAULT,cache::WRITEBACK);
		}
		__non_temporal process->calendar = 0;
		return 0;
	}
	ustd_t get_multi_from_pagetype(ustd_t pagetype){
		switch (pagetype){
			case pag::MIDPAGE:{ return 4096*512;}
			case pag::BIGPAGE:{ return 4096*512*512;}
			default:{ return 4096;}
		}
	}
	ustd_t get_pagetype_from_number(ustd_t number){
		number /= 4096;
		ustd_t pagetype = pag.SMALLPAGE;
		for (;; number; --pagetype){
			number /= 512;
		}
		++pagetype;
		if (pagetype > pag.SMALLPAGE){ --pagetype;}
		return pagetype;
	}
	/*
	Array of bits where each bit stands for a 4096 byte page, we have alignment requirements for the allocation, returned 1 doesnt negate the contents
	of the returned array meaning that you can get whatever is available (interactive syscalls?)
	*/
	ustd_t get_free_phys(void ** physrets, ustd_t pages_number, ustd_t pagetype){
		this->stream_init(NUH);
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
		this->stream_init(NUH);
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
		if (action == actions::SET){ this->stream_init(NUH);}
		ustd_t quota = get_multi_from_pagetype(pagetype);
		for (ustd_t i = 0; i < pages_number; ++i){
			ustd_t indexer = phys/8/4096;
			ustd_t bit = 1;
			for (ustd_t j = 0; j < quota; ++j){
				this->phys_ram_table[indexer] |= bit;
				if (action == actions::CLEAR){ this->phys_ram_table[indexer] ^= bit;}
				bit *= 2;
				if !(bit){ ++bit;}
			}
		}
		if (action == actions::SET){ __non_temporal this->calendar = 0;}
	}
	void * mem_alloc(void * pagetree, ustd_t pagetype, ustd_t pages_number){
		void * templ = memreq_template(pagetype,mode::RESERVE,0);
		void * ret = vmtree_fetch(pagetree,pages_number,pagetype);
		if (ret == -1){ return NULLPTR;}
		ulong_t * off = vmto_entry(pagetree,ret,&pagetype);
		for (ustd_t h = 0; h < pages_number; ++h){
			off[h] = templ;
		}
	}
	void * mem_map(void * pagetree, void * target, ustd_t pagetype, ustd_t pages_number, ustd_t cache){
		void * templ = memreq_template(pagetype,mode::MAP,cache);
		void * ret = vmtree_fetch(pagetree,pages_number,pagetype);
		if (ret == -1){ return NULLPTR;}
		ulong_t * off = vmto_entry(pagetree,ret,&pagetype);
		ustd_t addition = get_multi_from_pagetype(pagetype);
		for (ustd_t h = 0; h < pages_number; ++h){
			off[h] = templ | ((target+addition*h)<<13);
		}
	}
	void mem_free(void * pagetree, void * vm, ustd_t pages_number, ustd_t pagetype){	//you know that running this on fucking mapped pages marks teh end?
		void * phys[pages_number];
		if (deallocate_vm(get_process_object(NUH),phys,vm,pages_number)){get_process_object(NUH)->sigset |= signals::SIGSEGV; return;}
		manipulate_phys(phys,pages_number,pagetype,CLEAR);
		void * templ = memreq_template(pagetype,mode::RESERVE,cache::WRITEBACK);
		ulong_t off = vmto_entry(vm);
		for (ustd_t h = 0; h < pages_number; ++h){		//so theen this is basically 512 every single time
			off[h] = templ;
		}
	}
	//you can free the code you were executing or whatever, not my problem::
	void mem_unmap(void * pagetree, void * vm, ustd_t pages_number, ustd_t pagetype){
		ustd_t trash;
		ulong_t off = vmto_entry(pagetree,vm,&trash);
		void * templ = memreq_template(pagetype,mode::RESERVE,cache::WRITEBACK);

		for (ustd_t h = 0; h < pages_number; ++h){
			if (off[h] & 4){ return 1;}									//supervisor...
			if (off[h]<<53>>61 != mode::MAP){ get_process_object(NUH)->sigset |= SIGSEGV; return 2;}
		}
		for (ustd_t h = 0; h < pages_number; ++h){
			off[h] = templ;			//this is kind of bad though, because it lets you overwrite stuff like gdt or whatever, masssively dangerous.
		}
	}
	void * mem_realloc(void * pagetree, void * vm, ustd_t oldpages, ustd_t newpages, ustd_t pagetype){	//"Keep your pointer, son. I will take it from here."
		if (oldpages > newpages){return -1;}
		void * phys[newpages];
		if (deallocate_vm(pagetree,phys,vm,oldpages)){
			get_process_object(NUH)->sigset |= signals::SIGSEGV;}
		vm = vmtree_fetch(pagetree,newpages,pagetype);				//reminder to make the top of the possible address space reserved
		if (vm == -1){ return -2;
		if (get_free_phys(&phys[oldpages-1],newpages-oldpages,pagetype)){ return -3;}
		void * templ = memreq_template(pagetype,mode::RESERVE,cache::WRITEBACK);
		ulong_t off = vmto_entry(pagetree,vm);
		for (ustd_t h = 0; h < newpages; ++h){
			off[h] = templ | phys[h]<<13;
		}
		return vm;
	}
	void * mem_remap(void * pagetree, void * vm, ustd_t oldpages, ustd_t newpages, ustd_t pagetype){
		if (oldpages > newpages){return -1;}
		void * phys[newpages];
		if (deallocate_vm(pagetree,phys,vm,oldpages)){					//this is going to segmentation fault no matter what you do
			get_process_object(NUH)->sigset |= signals::SIGSEGV; return -1;}
		vm = vmtree_fetch(pagetree,newpages,pagetype);
		if (vm == -1){
			return -2;}
		void * templ = memreq_template(pagetype,mode::RESERVE,cache::WRITEBACK);
		ulong_t off = vmto_entry(pagetree,vm);
		ustd_t addition = get_multi_from_pagetype(pagetype);
		for (ustd_t h = 0; h < newpages; ++h){
			off[h] = templ | (phys+addition*h)<<13;
		}
		return vm;
	}
	void * exchange_swap(ulong_t * entry, void * diskpos, ustd_t swap_number){
		ulong_t new_entry = (entry*)<<55>>55;		//preserving the bottom 9 bits
		void * ret = (entry*)<<5>>18;
		entry* = diskpos | new_entry | 3<<10 | swap_number<<57;		//SWAPPED setting
		return ret;
	}
	/*
	This swaps out to disks whatever pages have been marked as reserved in a process' pagetree
	*/
	void swap_process(void){
		DisksKing * dking = get_disksking_object(NUH);
		Process * process = get_process_object(NUH);
		Virtual_fs * vfs = get_vfs_object(NUH);
		Kingmem * mm = get_kingmem_object(NUH);

		process->stream_init(NUH);

		ulong_t * pointer = process->pagetree;
		ustd_t interval = 512;
		ustd_t scaling = 512;
		ustd_t pagetype = 0;
		for (ustd_t i = 0; 1; ++i){
			if (i == scaling*interval){						//the ending conditions
				pointer = &pointer[i];
				scaling *= 512;
				if (scaling < get_multi_from_pagetype(pag::LEV4){ break;}
				i = 0;
			}
			if !(mm->sizeof_ram/mm->used_memory < 2){ break;}			//
			if (scaling == 512*512*512){						//the swapping conditions
				if !(pointer[i] & 1<<56){
					continue;}
			}else{
				if !(pointer[i] & 128){
					continue;}
			}									//
												//the swapping code
			void * diskpos = NULLPTR;
			for (1; diskpos == 0; ++part_counter){
				diskpos = dking->partitions[part_counter]->shm->pool_alloc(get_multi_from_pagetype(pagetype)/4096);
				CONDITIONAL_BLOOD_LIBEL(part_counter < dking->partitions->length);
				++swap_counter;
			}
			void * phys = exchange_swap(pointer[i],diskpos,swap_counter);
			manipulate_phys(phys,1,pagetype,actions::SET);
		}
		__non_temporal process->calendar = 0;
	}
	__attribute__((interrupt)) void fault_handler(void){
		enter_ringzero(NUH);
		Process * process = get_process_object(NUH);
		Kingmem * mm = get_kingmem_object(NUH);
		void * vm = __asm__("mov %%cr2,%%rax\n\t");
		ustd_t pagetype;
		ulong_t * entry = mm->vmto_entry(process->pagetree,vm,&pagetype);		//MASSIVE DANGER read the binary for how pagetype gets passed, if shit is grim use Kingmem (this is locked anyway)
		if (entry & 1){		//present bit, checking wether hard or soft fault
			mm->softfault_handler(entry);
		}
		else{
			mm->hardfault_handler(entry);
		}
	}
	void softfault_handler(Process * process, void * entry, ustd_t pagetype){
		if ((entry*)<<61>>63){	//supervisor
			process->sigset |= signals::SIGSEGV;
		}
		switch(entry* <<(64-12))>>53)
 			case mode::RESERVE:{
				void * phys = malloc(1,pagetype);
				entry* |= phys;
			break;}
			case mode::FILE:{				//see SYS_FMAP
				ustd_t desc = (entry*)<<18>>32;
				ustd_t page = (entry*)<<1>>(48+1);

				File * file = &vfs->descriptions[process->descs->pool[desc]->findex];
				entry* = vfs->load_page(get_disksking_object(NUH),file,page) | memreq_template(pagetype,mode::MAP,file->cache,file->executable);
			break;}
			case mode::SEGFAULT:{
				process->sigset |= signals::SIGSEGV;
			break;}
		}
	}
	void hardfault_handler(Process * process, void * entry, ustd_t pagetype){
		if (pagetype != pag::SMALLPAGE){++bit;}
		if ((entry*<<(64-12))>>54 == mode::SWAPPED){
			void * phys = malloc(1,pagetype);
			ustd_t ct = entry*<<1>>(57+1);
			Disksking * dking = get_disksking_object(NUH);
			for (ustd_t i = 0; i < dking->partitions->length; ++i)}
				if (dking->partitions->ckarray[i] == 0){ continue;}
				if (dking->partitions->pool[i]->signature == SWAP){
					--ct;
				}
				if (ct == 0){ dking->raw_read(entry*<<1>>(57+1)),entry*<<17>>30;
				break;}
			}
			if (ct){ free(phys,1,pagetype);
				process->sigset |= SIGKILL;
				return;
			}

			entry* = this->memreq_template(pagetype,RESERVE,WRITEBACK,0) | (phys<<5>>18);	//userspace means writeback is fit.
		}
		else{process->sigset |= SIGSEGV;}
	}
	void recursive_free(Process * process, ustd_t pagetype, uint64_t * pointer){
		for (ustd_t i = 0; i < 512; ++i){
			if !(pointer[i] & 1<<56){ continue;}				//if the page was not in use ignoring

			switch (pointer[i]<<(64-9)>>55){				//switch statement on the modes..., map gets ignored because whatever
				case SEGFAULT:{ break;}
				case RESERVE:{
					mem_free(process->pagetree,entryto_vm(process->pagetree,&pointer[i],1,pagetype));
				break;}
				case FILE:{
					Virtual_fs * vfs = get_vfs_object(NUH);
					File * file = &vfs->descriptions[process->descs->pool[pointer[i]<<1>>(48+1)]->findex];
					void ** dub = file->mem->pool;
					void * comp = pointer[i]<<17>>30;
					for (ustd_t k = 0; k < file->mem->length; ++k){	//sorting for which page we are even pointing at, not enough bits...
						if (dub* == comp){
							--file->mem->ckarray[k];
						break;}
					}
				break;}
			}
			if (pagetype == pag::SMALLPAGE){continue;}			//ignoring directory checks...
			if !(pointer[i] & 128){						//calling recursion on not-empty page directory
				recursive_free(process,pagetype+1,&pointer[i]);}
		}
	}
};
