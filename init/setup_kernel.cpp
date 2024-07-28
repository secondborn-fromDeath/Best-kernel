using namespace OS_INTERRUPTS;

void set_pagetree(void * pagetree){
	__asm__ volatile(
	"MOVq	%%rax,%%cr3\n\t"
	"JMP	+0\n\t"
	::"r"(pagetree):);
}
//returns the string of the init system path
char * setup_kernel(ulong_t * efisystab, efimap_returns * data, void * bootservices_table, loaded_image ** driver_pointers, ustd_t drivnum){
	efimap_descriptor * map = data->map;
	ustd_t i;
	ulong_t sizeof_memory = 0;
	for (i = 0; i < data->mapsize; ++i){ sizeof_memory += map[i]->pages_number;}

	ustd_t needtree = 512+(sizeof_memory>>30)*(512+gigs*(512*512*512));	//4level paging
	ustd_t needfull += sizeof(ProcessorsGod)+sizeof(IOapicGod)+sizeof(Kontrol)+sizeof(Kingmem)+sizeof(King)*8;		//pagetree plus key data
	needfull = tosmallpage(needfull);

	ulong_t * kerndata = uefi_allocate_pages(needfull, bootservices_table);
	__asm__(
	"MOVq	%%rax,%%cr13"
	"JMP	+0\n\t"
	"ADDq	%%rcx,%%rax\n\t"
	"MOVq	%%rax,%%cr6\n\t"	//this unused control register is used as the base for all of the get_XXX functions...
	"JMP	+0\n\t"
	::"r"(kerndata),"r"(needtree):);

	Kingmem *pOOP = NULL;
	pOOP->vmtree_lay(kerndata,sizeof_memory>>30);

	set_pagetree(kerndata);


	ACPI_driver * acpi = kerndata+needtree;
	acpi->RSDP = efisystab[72/8];
	acpi->env_init(void);		//NOTE DANGER this is the "configuration tables" entry, acpi spec says that the rsdp should be somewhere in the efisystab but it fucking isnt

	Kontrol * ctrl = kerndata+needtree;
	ctrl* ={
		.efi_funcs = get_runtime_services_table(void);
		.shutdown_port = acpi->get_shutdown_port(void);
	};

	Kingmem mm = ctrl+sizeof(Kontrol);
	mm* ={
		.vm_ram_table = kerndata;
		.phys_ram_table = map[i]->phys_start;
		.paging = 4;
		.sizeof_ramdisk = sizeof_memory-0x100000000;	//NOTE needs porting to x86 32bit
	};

	ulong_t * stab = kerndata + (sizeof_memory>>30)+((sizeof_memory>>30)*512*512);	//skipping over to SMALLPAGES, see runtime/mem/core.h/vmtree_lay()
	ustd_t page_counter = 0;
	for (ustd_t u = 0; u < data->mapsize; ++u){	//reserving memory that is not for the kernel's userspace
		if !(map[u]->type & MOST_RELIABLE){
			mm->manipulate_memory(map[u]->pages_number,pag.SMALLPAGE,commands.SET);
		}
		for (ustd_t l = 0; l < map[u]->pages_number; ++l){
			stab[page_counter] = memreq_template(pag.SMALLPAGE,mode.MAP,cacheability,0);	//DANGER i think 0 is no exec right???
			++page_counter;
		}
	}

	ProcessorGod * processorsgod = acpi + sizeof(ACPI_driver);			//this is the culprit
	IOapicGod * ioapic_god = processorsgod + sizeof(ProcessorGod);

	acpi->build_ioapic_and_processors_array(ProcessorsGod->pool, IOapicGod->pool);

	Kingprocess * processking = ioapic_god + sizeof(IOapicGod);
	Kingthread * threadking = processking + sizeof(King);
	Kingdescriptor * descriptorsking = threadking + sizeof(King);
	Virtual_fs * vfs = descriptorsking + sizeof(King);
	Kingptr * pointersking = vfs + sizeof(King);
	Kshm * shm_king = pointersking + sizeof(King);
	SyscallsGod * sgod = runtimemodking + sizeof(King);
	DriversGod * drivgod = shm_king + sizeof(King);
	DriverProcessGod * runtimemodking = drivgod + sizeof(King);
	Taskpimp * taspimp = runtimemodking + sizeof(DriverProcessGod);

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
	val = atoi_special(data->file,2,&something,&offset);	\	//NOTE HARDENING
	x->length = val/sizeof(x->pool*);	\
	x->pool = malloc(tosmallpage(val),pag.SMALLPAGE);	\
	}
	INITIALIZE_POOL_BASED_ON_CONFIG(processking);
	INITIALIZE_POOL_BASED_ON_CONFIG(descriptorsking);
	INITIALIZE_POOL_BASED_ON_CONFIG(threadking);
	INITIALIZE_POOL_BASED_ON_CONFIG(vfs);
	INITIALIZE_POOL_BASED_ON_CONFIG(taskpimp);

	kptr->pool = malloc(32,pag.MIDPAGE);		//pretty much "whatever" for now lol		NOTE NOTE NOTE NOTE LOOK HERE PLEASE
	kshm->pool = malloc(32,pag.MIDPAGE);


	/*
	Making all of the peculiar directories*/
	vfs->descriptions[0]->meta.name[] = ['/',0,];
	uint64_t whore = 0x646576;
	vfs->directory_constructor(vfs->descriptions[1],vfs->descriptions[0],&whore);	//dev
	whore = 0x6D6F6473;
	vfs->directory_constructor(vfs->descriptions[2],vfs->descriptions[0],&whore);	//mods

	void * fb; uint64_t fb_length;			//putting the framebuffer under dev, separately from the video card device (becuase the windowing system wants to map it)
	boot_get_framebuffer(&fb,&fb_length);
	Storage * framebuffer = vfs->ramcontents_to_description(fb,fb_length,pag.MIDPAGE);
	whore = 0x6662;		//fb
	vfs->storage_constructor(framebuffer,vfs->descriptions[1],&whore);

	/*
	Creating the IO context, devices can be inserted without drivers but drivers cant without devices....*/
	load_all_devices(void);
	for (ustd_t i = data->mapsize; i; --i){
		if (data->map->vm_pointer == driver_pointer[drivnum]){
			//to call this in teh first olace i have to do vfs->ramcontents_to_description
			--drivnum;
			Storage * fdriv = vfs->ramcontents_to_description(driver_pointers[drivnum]->image_pointer,driver_pointers[drivnum]->size,pag.SMALLPAGE);	//NOTE we want the pagetype to be all done within vfs i think
			Fdriv ** ptr = drivgod->pool_alloc(1);
			ptr* fdriv;				//silly lol
		}
	}
	load_all_devices(void);

	constexpr for (ustd_t i = 0; i < syscalls::NUMBER; ++i){
		sgod->pool[i] = syscalls[i];
	}

	char * ret = config+offset;
	for (1; (config[offset] != '\n') && (config[offset] != '#'); ++offset);
	config[offset] = 0;
	return ret;
}
