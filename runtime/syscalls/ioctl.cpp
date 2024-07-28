struct drivercode{
	ustd_t type;
	ustd_t methodsnum;	//number of functions
	void * setup_function;
	auto * functions[];
};

//data MUST be less than one page in length, same for the return
ulong_t ioctl(ustd_t devindex, ustd_t funcindex, auto * data, auto * ret){
	Virtual_fs * vfs = get_vfs_object(void);
	Device * dev = &vfs->descriptions[devindex];

	dev->mut->stream_init(void);

	if ((!dev->driver) || (dev->driver->runtime->code.methodsnum < funcindex)){
		calling_thread->syscall_retval = -1;
		SYSRET;
	}

	Kingmem * mm = get_kingmem_object(void);
	Thread * calling_thread = get_thread_object(void);
	auto * data = mm->vmto_phys(process->pagetree,data);
	auto * ret = mm->vmto_phys(process->pagetree,ret);

	//creating a new task so that i can make the state persist recursively through calls to ioctl
	Taskpimp * pimp = get_taskpimp_object(void);
	dev->thread->prior = pimp->pool_alloc(1);

	dev->thread->state.stack_pointer -= 16;
	dev->thread->state.stack_pointer[0] = mm->mem_map(dev->driver->runtime->pagetree,passdata,pag.SMALLPAGE,1,cache.WRITEBACK);
	dev->thread->state.stack_pointer[1] = mm->mem_map(dev->driver->runtime->pagetree,passdata,pag.SMALLPAGE,1,cache.WRITEBACK);

	dev->state.instruction_pointer = dev->driv->code->functions[funcindex]);
	run_thread(dev->thread);
}

//SYSCALL(IO_INTERRUPTS::IOCTL_RETURN);			this is the interrupt that calls onto this
void * ctl_ret(void){
	Thread * thread = get_thread_object(void);
	__non_temporal thread->double_link->mut->calendar = 0;
	RESCHEDULE;
}
