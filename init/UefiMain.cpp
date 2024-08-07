void cli(void){__asm__ volatile("CLI\n\t");}
void sti(void){__asm__ volatile("STI\n\t");}

void processor_init(void){
	cli(NUH);
	setup_MSRS(NUH);
	ensure_interrupts_mode(NUH);
}

ustd_t UefiMain(void * image_handle, void * efisystab){
	processor_init(NUH);
	if !(test_ps2_controller(NUH)){ shutdown(NUH);}
	void * boottab = get_bootservices_table(efisystab);
	image_loaded * config;
	ustd_t drivers_number = fetch_drivers_basics(boottab,&config);
	image_loaded * driver_pointers[drivers_number];				//...stack overflow in a bootloader? lel
	image_loaded * info_pointers[drivers_number];


	char * initsys = setup_kernel(image_handle,efisystab,boottab,&mapdata);	//new pagetree was set...
	void * pagetree = get_ringzero_pagetree(NUH);
	Pci * pci = get_pci_object(NUH);
	enact_IO_context(vmto_phys(pagetree,driver_pointers),vmto_phys(pagetree,mapdata));
	setup_gdt(NUH);
	setup_idt(NUH);
	ustd_t len, init_index;
	vfs->open(initsys,0,&len,&index);
	exec(init_index);
	ustd_t root = vfs->mount(root_disk,root_partition_pos);
	exit_bootservices(mapdata->mapkey,boottab);
	assign_interrupt_vectors(NUH);
	sti(NUH);
	initialize_brothers(NUH);
	routine(NUH);
}
