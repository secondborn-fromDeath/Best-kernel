void setup_memory(efimap_returns * data){
	init_PAT(void);	//TODO

	efimap_descriptor * map = data->map*;
	ustd_t i;
	ulong_t sizeof_memory = 0;
	for (i = 0; i < data->mapsize*; ++i){ sizeof_memory += map[i]->pages_number;}

	ustd_t needtree = 512+(sizeof_memory>>30)*(512+gigs*(512*512*512));	//4level paging
	ustd_t needfull += sizeof(ProcessorsGod)+sizeof(IOapicGod)+sizeof(Kontrol)+sizeof(Kingmem)+sizeof(King)*8;		//pagetree plus key data

	if (needfull%4096){ needfull = (needfull/4096+1)*4096;}	//losslessly rounding to pagesize
	else { needfull /= 4096;}

	for (i = 0;  i < data->mapsize*; ++i){
		if !(map[i]->pages_number < need){
			break;
		}
	}
	if (map[i]->pages_number < need){
		//uefi turn machine off
	}
	ulong_t * kerndata = map[i]->phys_start;
	__asm__(
	"MOV	%%rax,%%cr6\n\t"	//NOTE unused control register
	"JMP	+0\n\t"
	::"r"(kerndata):);

	Kingmem * mm = NULL;
	mm->vmtree_lay(kerndata,sizeof_memory>>30);

	Kontrol * ctrl = kerndata+needtree;
	ctrl* ={
		.efi_funcs = get_runtime_services_table(void);
	};

	for (1;  i < data->mapsize*; ++i){
		if !(map[i]->pages_number < sizeof_memory/8){
			break;
		}
	}
	if (map[i]->pages_number < sizeof_memory/8){
		//uefi turn machine off
	}


	acpi->env_init(void);

	Kingmem mm = ctrl+sizeof(Kontrol);
	mm* ={
		.vm_ram_table = kerndata;
		.phys_ram_table = map[i]->phys_start;
		.paging = 4;
		.sizeof_ramdisk = sizeof_memory-0xFFFFFFFF;
		.shutdown_port = acpi->get_shutdown_port(void);
	};

	ulong_t * stab = kerndata + (sizeof_memory>>30)+(sizeof_memory>>30*512*512);	//skipping over to SMALLPAGES, see runtime/mem/core.h/vmtree_lay()
	ustd_t page_counter = 0;
	for (ustd_t u = 0; u < data->mapsize*; ++u){					//reserving memory in the kernel maps
		if !(map[u]->type & MOST_RELIABLE){
			mm->manipulate_memory(map[u]->pages_number,pag.SMALLPAGE,commands.SET);
		}
//		skipped forward, now incrasing and assigning with memreq_template
		//so basically i am going to have to decode the cache from the descriptor entries to my caching enumeration
		ustd_t cacheability = 0;
		if (map[u]->memtype & cache.WRITEBACK){;}
		else if (map[u]->memtype & cache.STRONG_UNCACHEABLE){ cacheability = cache.STRONG_UNCACHEABLE;}
		else if (map[u]->memtype & cache.WRITE_COMBINING){ cacheability = cache.WRITE_COMBINING;}
		else if (map[u]->memtype & cache.WRITE_THROUGH){ cacheability = cache.WRITE_THROUGH;}
		else if (map[u]->memtype & cache.WRITE_PROTECT){ cacheability = cache.WRITE_PROTECT;}

		for (ustd_t l = 0; l < map[u]->pages_number; ++l){
			stab[page_counter] = memreq_template(pag.SMALLPAGE,mode.MAP,cacheability,0);	//DANGER i think 0 is no exec right???
			++page_counter;
		}
	}

	ACPI_driver * acpi = mm + sizeof(Kingmem);
	ProcessorGod * prcgod = acpi + sizeof(King)*8;
	IOapicGod * ioapic_god = prcgod + sizeof(ProcessorGod);

	acpi->build_ioapic_and_processors_array(ProcessorsGod->pool, IOapicGod->pool);


	set_pagetree(kerndata);
	//TODO allocate memory for all of teh structures based on the sizeof memory and the command line (file??)
}
void sti(void){__asm__("STI\n\t");
}
ustd_t 64bit_start(void * image_handle, void * efisystab){
	efi_returns data;
	get_uefi_memmap(image_handle,efisystab,&data);
	setup_memory(&data);			//and the kernel data
	setup_gdt(void);
	setup_idt(void);
	exit_bootservices(data->mapkey*);
	POST_check(void);			//TODO acpi shutdown
	enumerate_devices(void);
	enable_apic(void);			//TODO
	assign_interrupt_vectors(void);		//TODO
	mount(root_disk,root_partition_pos);	//TODO
	sti(void);
	initialize_brothers(void);		//TODO
	bystring_exec(init_system);		//TODO wrap
	routine(void);
}
