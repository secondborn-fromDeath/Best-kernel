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
	ustd_t drivnum = initialize_drivers(boottab,driver_pointers);
	efimap_returns data; get_uefi_memmap(image_handle,efisystab,&data,boottab);
	char * initsys = setup_kernel(efisystab,boottab,&data,driver_pointers,drivnum);		//from here on out you can use the kernel memory allocation functions
	POST_check(void);
	setup_gdt(void);
	setup_idt(void);
	exit_bootservices(data->mapkey,boottab);
	assign_interrupt_vectors(void);
	sti(void);
	initialize_brothers(void);
	Virtual_fs * vfs = get_vfs_object(void);
	ustd_t root = vfs->mount(root_disk,root_partition_pos);
	ustd_t init_index = vfs->open(initsys,root);
	if (vfs->descriptions[init_index]->type == filetypes.DIRECTORY){ shutdown(void);}		//meaning stand was returned from recurse_directories, should probably log the error... somehow, though
	routine(void);
}
