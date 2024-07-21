void setup_kernel(efimap_returns * data, void * bootservices_table){
	efimap_descriptor * map = data->map;
	ustd_t i;
	ulong_t sizeof_memory = 0;
	for (i = 0; i < data->mapsize; ++i){ sizeof_memory += map[i]->pages_number;}

	ustd_t needtree = 512+(sizeof_memory>>30)*(512+gigs*(512*512*512));	//4level paging
	ustd_t needfull += sizeof(ProcessorsGod)+sizeof(IOapicGod)+sizeof(Kontrol)+sizeof(Kingmem)+sizeof(King)*8;		//pagetree plus key data
	tosmallpage(needfull);

	for (i = 0;  i < data->mapsize; ++i){
		if !(map[i]->pages_number < need){
			break;
		}
	}
	if (map[i]->pages_number < need){
		//uefi turn machine off
	}
	ulong_t * kerndata = map[i]->phys_start;
	__asm__(
	"MOV	%%rax,%%cr6\n\t"	//this unused control register is used as the base for all of the get_XXX functions...
	"JMP	+0\n\t"
	::"r"(kerndata+needtree):);

	Kingmem *pOOP = NULL;
	pOOP->vmtree_lay(kerndata,sizeof_memory>>30);


	ACPI_driver * acpi = kerndata+needtree;
	acpi->env_init(void);

	Kontrol * ctrl = kerndata+needtree;
	ctrl* ={
		.efi_funcs = get_runtime_services_table(void);
		.shutdown_port = acpi->get_shutdown_port(void);
	};

	for (1;  i < data->mapsize*; ++i){
		if !(map[i]->pages_number < sizeof_memory/8){
			break;
		}
	}
	if (map[i]->pages_number < sizeof_memory/8){
		//uefi turn machine off
	}

	Kingmem mm = ctrl+sizeof(Kontrol);
	mm* ={
		.vm_ram_table = kerndata;
		.phys_ram_table = map[i]->phys_start;
		.paging = 4;
		.sizeof_ramdisk = sizeof_memory-0xFFFFFFFF;
	};

	ulong_t * stab = kerndata + (sizeof_memory>>30)+((sizeof_memory>>30)*512*512);	//skipping over to SMALLPAGES, see runtime/mem/core.h/vmtree_lay()
	ustd_t page_counter = 0;
	for (ustd_t u = 0; u < data->mapsize; ++u){					//reserving memory in the kernel maps
		if !(map[u]->type & MOST_RELIABLE){
			mm->manipulate_memory(map[u]->pages_number,pag.SMALLPAGE,commands.SET);
		}
		//skipped forward, now incrasing and assigning with memreq_template	i am using the same config as uefi for caching since it is different than the x64 defualt (which would throw a GPE#)
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

	ProcessorGod * processorsgod = acpi + sizeof(ACPI_driver);			//this is the culprit
	IOapicGod * ioapic_god = processorsgod + sizeof(ProcessorGod);

	Kingprocess * processking = ioapic_god + sizeof(IOapicGod);
	Kingthread * threadking = processking + sizeof(King);
	Kingdescriptor * descriptorsking = threadking + sizeof(King);
	Virtual_fs * vfs = descriptorsking + sizeof(King);
	Kingptr * pointersking = vfs + sizeof(King);
	Kshm * shm_king = pointersking + sizeof(King);
	DriversGod * drivgod = shm_king + sizeof(King);
	DriverProcessGod * runtimemodking = drivgod + sizeof(King);

	/*
	ugly...
	basically I am extracting 4 values from the config file and assigning memory to the structures
	NOTE HARDENING		for now there are no integrity checks at all...
	*/
	ulong_t string[8];
        string[0] = 0x2F6B65726E2E6366;         //"/kern.cf"
	string[1] = NULL;
	loadfile_returns data;
	char * config = loadfile(string,bootservices_table,&data);
	char something = '#'|('\n'<<8);
	ustd_t offset = 0;			//updated throughout
	ustd_t val;

	#define INITIALIZE_POOL_BASED_ON_CONFIG(x){	\
	val = atoi_special(data->file,2,&something,&offset);	\
	x->length = val/sizeof(x->pool*);	\
	x->pool = malloc(tosmallpage(val),pag.SMALLPAGE);	\
	}
	INITIALIZE_POOL_BASED_ON_CONFIG(processking);
	INITIALIZE_POOL_BASED_ON_CONFIG(descriptorsking);
	INITIALIZE_POOL_BASED_ON_CONFIG(threadking);
	INITIALIZE_POOL_BASED_ON_CONFIG(vfs);

	//TODO figure something out for kingptr and kingshm, throw in sizeof_memory too......	drivergods are going to work without a startup, doesnt really matter


	acpi->build_ioapic_and_processors_array(ProcessorsGod->pool, IOapicGod->pool);

	set_pagetree(kerndata);
}
