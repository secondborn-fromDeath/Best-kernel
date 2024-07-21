void cli(void){__asm__("CLI\n\t");}
void sti(void){__asm__("STI\n\t");}

ustd_t UefiMain(void * image_handle, void * efisystab){
	cli(void);
	enable_A20(void);
	setup_MSRS(void);			//TODO
	enable_apic(void);			//TODO
	efimap_returns data;
	void * boottab = get_bootservices_table(efisystab);
	get_uefi_memmap(image_handle,efisystab,&data,boottab);
	setup_kernel(&data);
	POST_check(void);			//TODO acpi shutdown
	setup_gdt(void);
	setup_idt(void);
	enumerate_devices(void);		//must get done first
	setup_drivers(void);
	enumerate_devices(void);		//trust me
	assign_interrupt_vectors(void);		//TODO
	sti(void);
	exit_bootservices(data->mapkey*,boottab);
	mount(root_disk,root_partition_pos);
	initialize_brothers(void);		//TODO
	bystring_exec(init_system);		//TODO wrap
	routine(void);
}
