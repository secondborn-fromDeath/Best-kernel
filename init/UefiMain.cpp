void cli(void){__asm__("CLI\n\t");}
void sti(void){__asm__("STI\n\t");}

void processor_init(void){
	cli(void);
	setup_MSRS(void);
	ensure_interrupts_mode(void);
}

ustd_t UefiMain(void * image_handle, void * efisystab){
	processor_init(void);
	if !(test_ps2_controller(void)){ shutdown(void);}
	enable_A20(void);							//in uefi this *should* not be needed but whatever
	void * boottab = get_bootservices_table(efisystab);
	void * driver_pointers[16];
	initialize_drivers(boottab,driver_pointers);
	efimap_returns mapdata;
	get_uefi_memmap(image_handle,efisystab,&mapdata,boottab);
	char * initsys = setup_kernel(efisystab,boottab,&mapdata,driver_pointers,drivnum);		//from here on out you can use the kernel memory allocation functions
	POST_check(void);
	ustd_t len, init_index;
	vfs->open(initsys,0,&len,&index);
	exec(init_index);
	enact_IO_context(driver_pointers,mapdata);
	setup_gdt(void);
	setup_idt(void);
	ustd_t root = vfs->mount(root_disk,root_partition_pos);
	if (vfs->descriptions[init_index]->type == filetypes.DIRECTORY){ shutdown(void);}               //meaning stand was>
	exit_bootservices(mapdata->mapkey,boottab);
	assign_interrupt_vectors(void);
	sti(void);
	initialize_brothers(void);
	routine(void);
}
