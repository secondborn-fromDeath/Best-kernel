/*
Translates arguments to something kernelspacew palatable, calls the function from the driver's functions
*/

struct drivercode{
	ustd_t type;
	ustd_t methodsnum;	//number of functions
	void * setup_function;
	auto * functions[];
};

ulong_t ioctl(ustd_t drivindex, ustd_t funcindex, auto * data, auto * ret){
	Virtual_fs * vfs = get_vfs_object(void);
	Device * dev = &vfs->descriptions[drivindex];

	if (dev->code.methodsnum < funcindex){ return 1;}

	Kingmem * mm = get_kingmem_object(void);
	Process * process = get_process_object(void);
	mm->stream_init(void);
	void * backup = mm->vm_ram_table;
	mm->vm_ram_table = process->pagetree;
	auto * passdata = mm->vmto_phys(data);
	auto * passret = mm->vmto_phys(ret);
	mm->vm_ram_table = backup;
	__non_temporal mm->calendar = 0;

	(dev->driv->code->functions[funcindex])(passdata,passret);
	return 0;
}
