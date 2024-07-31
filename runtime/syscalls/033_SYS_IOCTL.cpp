struct drivercode{
	char name[];
	ustd_t typerec;		//number of types
	ustd_t type[];
	ustd_t methodsnum;	//number of functions
	void * setup_function;
	auto * functions[];
};

/*Data MUST be less than one page in length, same for the return, the file passed by userspace *cant* be bad.
on bad pointer i dont signal a segfault... because*/
ulong_t ioctl(ustd_t devindex, ustd_t funcindex, auto * data){
	Thread * calling_thread = get_thread_object(void);
	Virtual_fs * vfs = get_vfs_object(void);

	Device * dev = &vfs->descriptions[devindex];
	dev->mut->stream_init(void);

	CONDITIONAL_SYSRET(calling_thread,(dev->meta.mode & filemodes::ROOT_WRITE)&&(calling_thread->parent->owner_id != owner_ids::ROOT),1);
	CONDITIONAL_SYSRET(calling_thread,dev->driver->runtime->code.methodsnum < funcindex,2);

	Kingmem * mm = get_kingmem_object(void);

	auto * passdata = mm->vmto_phys(dev->thread->father->pagetree,data);
	CONDITIONAL_SYSRET(calling_thread,!passdata,3);

	//...so that i can make the state persist through recursive calls to ioctl
	dev->thread->prior = calling_thread;

	dev->thread->state.stack_pointer -= 16;
	ulong_t * something = vmto_phys(dev->thread->father->pagetree,dev->thread->state.stack_pointer);
	something[0] = mm->mem_map(dev->driver->runtime->pagetree,passdata,pag.SMALLPAGE,1,cache.WRITEBACK);
	something[1] = NULLPTR;

	dev->state.instruction_pointer = dev->driv->code->functions[funcindex]);
	run_thread(dev->thread);
}

void SYS_IOCTL(char * device_path, ustd_t funcindex, auto * data){
	Thread * thread = get_thread_object(void);
	Process * process = thread->parent;
	Virtual_fs * vfs = get_vfs_object(void);
	Kingmem * mm = get_kingmem_object(void);

	char * true_path = mm->vmto_phys(process->pagetree,device_path);		//this does not account for the fucking offsets. oh my god.
	for (ustd_t i = 0; 1; ++i){ if (i == 4095){ CONDITIONAL_SYSRET(thread,truepath[i] == 0,1); break;}}

	ustd_t len;
	ustd_t index;
	CONDITINOAL_SYSRET(thread,vfs->open(true_path,0,&len,&index),3);	//passing root

	ioctl(index,funcindex,data);
}


//SYSCALL(IO_INTERRUPTS::IOCTL_RETURN);			this is the interrupt that calls onto this
void * ctl_ret(void){
	Kingmem * mm = get_kingmem_object(void);
	Thread * thread = get_thread_object(void);
	if (thread->called){
		__non_temporal thread->taken = 0;
		set_thread(thread->prior);
		load_state(thread->prior);
		LONGJUMP(get_thread_object(void)->state.instruction_pointer);
	}
	mm->mem_unmap(thread->parent->pagetree,thread->state.stack_pointer[0],1,pag.SMALLPAGE);
	mm->mem_unmap(thread->parent->pagetree,thread->state.stack_pointer[1],1,pag.SMALLPAGE);
	thread->state.stack_pointer += 16;
	__non_temporal thread->taken = 0;
	set_thread(thread->prior);
	thread->prior->sys_retval = 0;
	SYSRET;
}
#define INIT_KERNEL_THREAD(kernel, threadking){	\
kernel = threadking->pool_alloc(1);	\
set_thread(threadsking->pool_alloc(1));		\
kernel->type = thread_types::KERNEL;	\
store_state(void);	\
}	\

