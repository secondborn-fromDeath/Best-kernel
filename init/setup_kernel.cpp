void * get_ringzero_pagetree(NUH){
	__asm__ volatile(
	"MOVQ	%%cr13,%%rax\n\t"
	"RET\n\t"
	:::);
}
struct boot_translations{
	void * imagehandle;
	void * efisystab;
	void * kerntree;		//comes bundled with kerndata
	uint64_t mapkey;
	void * phys_ram_table;
	ulong_t sizeof_memory;
	image_loaded * configuration;
};
//returns the string of the init system path
boot_translations * setup_kernel(void * image_handle, ulong_t * efisystab, void * bootservices_table){
	ustd_t needtree = 512*512*4096;						//2mb, 4level paging
	constexpr ustd_t needfull = needtree+FULLSINGLETONS;			//pagetree plus key data
	needfull = tosmallpage(needfull);

	void * kerntree = uefi_allocate_pages(needfull+1,bootservices_table);
	Kingmem *pOOP = kerntree+needfull*4096;
	volatile boot_translations * trans = pOOP+sizeof(Kingmem);


	for (ustd_t i = 0; i < mapdata->mapsize; ++i){
		trans->sizeof_memory += map[i]->pages_number;
	}

	void * virtu_ramtab = uefi_allocate_pages(sizeof_memory/4096/8,bootservices_table);

        efimap_returns mapdata;
        get_uefi_memmap(image_handle,efisystab,&mapdata,boottab);
	efimap_descriptor * map = mapdata->map;

	for (ustd_t i = 0; i < mapdata->mapsize; ++i){
		if (map[i]->virt_start == kerntree){ trans->kerntree = map[i]->phys_start;}
		else if (map[i]->virt_start == efisystab){ trans->efisystab = map[i]->phys_start;}
		else if (map[i]->virt_start == virtu_ramtab){ trans->phys_ram_table = map[i]->phys_start;}
	}

	__asm__ volatile(		//control regs used in the get_whatever functions, see initialize_brothers for a full control register reference
	"MOVQ	%%rax,%%cr13"
	"JMP	+0\n\t"
	"ADDQ	%%rcx,%%rax\n\t"
	"MOVQ	%%rax,%%cr6\n\t"
	"JMP	+0\n\t"
	::"r"(phystree),"r"(needtree):);

	pOOP->vmtree_lay(kerntree);
	pOOP->phys_mem_array = uefi_allocate_pages(sizeof_memory/4096/8,bootservices_table);

	/*
	ugly...
	basically I am extracting 4 values from the config file and assigning memory to the structures
	*/
	ulong_t string[8];
        string[0] = 0x2F6B65726E2E6366;         //"/kern.cf"
	string[1] = NULL;
	trans->configuration = uefi_loadfile(string,bootservices_table,image_handle);

	get_uefi_memmap(image_handle,efisystab,&mapdata,boottab);		//updating the key
	map = memmap->map;
	trans->mapkey = memmap->mapkey;

	for (ustd_t i = 0; i < mapdata->mapsize; ++i){
		using namespace uefi_memory::attributes;	//abilities of the block device
		using enum uefi_memory::boot_types;		//what it is being used for
		//using enum cache;				//cache values translation
		if !(map[i]->attributes & __CONVENTIONAL__){
			pOOP->manipulate_phys(map[i]->phys_start,map[i]->pages_number,pag::SMALLPAGE,actions::SET);
			continue;
		}

		ustd_t cache = cache::WRITEBACK;
		if (map[i]->memtype & STRONG_UNCACHEABLE){
				cache = cache::STRONG_UNCACHEABLE;}
		else if (map[i]->memtype & WRITE_COMBINING){
				cache = cache::WRITE_COMBINING;}
		else if (map[i]->memtype & WRITETHROUGH){
				cache = cache::WRITETHROUGH;}
		else if (map[i]->memtype & WRITEBACK){
				cache = cache::WRITEBACK;}
		else if (map[i]->memtype & WEAK_UNCACHEABLE){
				cache = cache::WEAK_UNCACHEABLE;}
		else if  (map[i]->memtype & WRITEPROTECT){
				cache = cache::WRITEPROTECT;}
		pOOP->map_identity(kerndata,map[i]->phys_start,map[i]->pages_number,cache,1);
	}

	boot_translations * eise;	//setting the pagetree with assembly to get around the compiler's limitations
	__asm__ volatile(
	"MOVQ	%%rax,%%cr8\n\t"
	"JMP	+0\n\t"
	"ADDQ	%%rcx,%%rax\n\t"
	:"=r"(eise):"r"(trans->kerntree),"r"(needfull*4096+sizeof(Kingmem)):);

	return finalize_kernel_data(eise);
}

boot_translations * finalize_kernel_data(boot_translations * data){
	using namespace singletons;

	void * st_base = data->kerntree+512*512*4096;

	ACPI_driver * acpi = st_base+ACPI_OFFSET;
	acpi->env_init(efisystab[76/8]);		//EFI_CONFIGURATION_TABLE or somethign

	Kontrol * ctrl = st_base+KONTROL_OFFSET;
	ctrl* ={
		.efisystab = data->efisystab;
		.efi_funcs = get_runtimeservices_table(data->efisystab);
//		.shutdown_port = acpi->get_shutdown_port(NUH);
		.shutdown_port = 0xB000;			//NOTE QEMU
	};

	Kingmem mm = st_base+KINGMEM_OFFSET;
	mm* ={
		.phys_ram_table = data->phys_ram_table;
		.used_memory = 0;				//only counting what userspace processes use
	};


	ProcessorGod * processorsgod = st_base+PROCESSOR_OFFSET;
	IOapicGod * ioapic_god = st_base+IOAPIC_OFFSET;

	acpi->build_ioapic_and_processors_array(ProcessorsGod->pool, IOapicGod->pool);

	Kingprocess * processking = st_base+PROCESS_OFFSET;
	Kingthread * threadking = st_base+THREAD_OFFSET;
	Kingdescriptors * kdesc = st_base+DESCRIPTOR_OFFSET;
	Virtual_fs * vfs = st_base+VFS_OFFSET;
	Kingptr * pointersking = st_base+POINTER_OFFSET;
	DriversGod * drivgod = st_base+DRIVERS_OFFSET;
	QuirksGod * qgod = st_base+QUIRKS_OFFSET;
	Modules_removal_stack * modrm_sp = st_base+MODRM_STACK;
	Disksking * dking = st_base+DISKSKING_OFFSET;
	Pci * pci = st_base+PCI_OFFSET;

	processking->type = hashtypes::SINGLE_POINTER;
	threadking->type = hashtypes::SINGLE_POINTER;
	kdesc->type = hashtypes::SINGLE_POINTER;
	vfs->type = hashtypes::SINGLE_POINTER;
	pointersking->type = hashtypes::SINGLE_POINTER;
	drivgod->type = hashtypes::DOUBLE_POINTER;
	qgod->type = hashtypes::DOUBLE_POINTER;
	modrm_sp->type = hashtypes::DOUBLE_POINTER;
	dking->type = hashtypes::DOUBLE_POINTER;
	pci->env_init(NUH);

	ushort_t something = '\n'|('#'<<8);
	char * image_base = mm->vmto_phys(data->kerntree,data->configuration->image_base);
	uint64_t val;
	uint64_t offset = 0;
	#define INITIALIZE_POOL_BASED_ON_CONFIG(x,y){	\
	val = atoi_special(image_base,2,&something,&offset);	\	//NOTE HARDENING
	x->length = val/sizeof(x->pool*);	\
	x->pool = malloc(tosmallpage(val)*y,pag::SMALLPAGE);	\
	}
	INITIALIZE_POOL_BASED_ON_CONFIG(processking,1);
	INITIALIZE_POOL_BASED_ON_CONFIG(threadking,2);	//because there are more threads than just application threads
	INITIALIZE_POOL_BASED_ON_CONFIG(vfs,1);

	kptr->pool = malloc((vfs->length+processking->length)/8,pag.MIDPAGE);	//if vfs is 8mill files then this is 8mill*512 which is 4bill, meaning 4 gigabytes...wtf


	/*
	Making all of the peculiar directories*/
	Directory * ROOT = &vfs->descriptions[0];
	ROOT->meta.name[] = ['/',0,];
	uint64_t whore = 0x646576;
	vfs->directory_constructor(vfs->descriptions[1],ROOT,&whore);			//dev
	whore = 0x6D6F6473;
	vfs->directory_constructor(vfs->descriptions[2],ROOT,&whore);			//mods

	//returning the name of the init system
	char * ret = image_base+offset;
	for (1; (image_base[offset] != '\n') && (image_base[offset] != '#'){ ++offset;}
	image_base[offset] = 0;
	return ret;
}
