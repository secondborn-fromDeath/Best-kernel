using namespace signals;

extern void store_state(Thread * thread){				//these also do the pagetree
#define ENTER_RINGZERO LONGJUMP(&enter_ringzero);
extern void leave_ringzero(struct interrupt_stack in);
extern void enter_ringthree(Thread * thread);
//void double_fault_store_state(void);				//... LONGTERM


	void run_ringthree(Thread * thread){
		set_ipi_mode(get_selfipi_mask(NUH));
		set_task_priority(0);
		cli(NUH);			//boot lol
		thread->state::accumulator = thread->sys_retval;
		if (thread->type == APPLICATION) {schedule_timed_interrupt(SCHEDULER_INTERRUPT_TIMER,EIGHT);}	//undefined and random value, see drivers/interrupts.cpp
		sysret_load_stack(thread);
		leave_ringzero(thread);
	}
	void run_thread(Thread * thread){
		set_thread_object(thread);
		run_ringthree(thread);
	}
	void help_out(void){
		Processor * processor = get_processor_object(NUH);
		run_thread(&get_threadsking_object(NUH)->pool[processor->current_thread]);
	}
	void getnext_thread(Processor * processor){
		ThreadsKing * tking = get_threadsking_object(NUH);
		tking->stream_init(NUH);
		ulong_t current_micros = gettime_millis(NUH);
		for (ustd_t i = processor->current_thread+1; i != processor->current_thread; ++i){
			if (i == tking->length){ i = 0;}

			if (tking->pool[i]->parent->sigset & SIGKILL){
				TKILL(thread);
				if !(tking->pool[i]->parent->workers->count){
					Processking * processking = get_processking_object(NUH);
					PKILL(tking->pool[i]->parent,1);	//gets all of our memory back and frees the spot in the process pool, the memset should be in there
				}
			}

			if (tking->pool[i]->type == threadtypes::DRIVER){ tking->pool_free(&tking[i],1);}	//see modinsert
			if !(tking->pool[i]->taken){
				processor->current_thread = i;
				__non_temporal tking->calendar = 0;
				return;
			}
			if (tking->pool[i]->target_micros < current_micros){
				continue;}
		}
		__non_temporal tking->calendar = 0;
		processor->current_thread = -1;
		halt(NUH);
	}
	ustd_t signal_handler(Thread * thread, ustd_t index, ustd_t signal){
		switch (index){
			case SIGKILL:{	TKILL(thread); return 1;}
			case SIGSTOP:{	return 1;}
			case SIGCONT:{	thread->sigset |= SIGSTOP; thread->sigset ^= SIGSTOP; return 0;}
			default: {
				if (thread->sigmask & signal){
					return 0;
				}
				if (thread->sighandlers[index]){
					thread->state->instruction_pointer = thread->sighandlers[index];	//this includes sigterm
					thread->sigset |= SIGKILL;
					run_ringthree(thread);
				}
				return 0;
			}
		}
	}
	/*
	Checks both the thread local signals and those of the process*/
	void check_signals(Thread * thread){
		ustd_t g = 1;
		for (sig_t i = 0; i < MAXSIG; ++i){
			if (thread->sigset & g){
				if (signal_handler(thread,i,g)){ RESCHEDULE;}
			}
			g*=2;
		}
		Process * process = thread->parent;
		for (ustd_t g = 0; i < MAXSIG; ++g){
			if (process->sigset & g){
				switch (g){
					case SIGKILL:{ PKILL(thread->parent); return;}
					case SIGCONT:{ process->sigset |= SIGSTOP; process->sigset ^= SIGSTOP;}
				}
			}
		}
	}
	void get_some_help(void){
		ProcessorsGod * processors = get_processorsgod_object(NUH);
		for (ustd_t i = 0; i < get_processors_number(NUH); ++i){
			Processor * brother = &processors->pool[i];
			if !(brother->executed_threads){
				if (brother->online_capable){
					Processor * struggler = get_processor_object(NUH);
					ustd_t backup = struggler->current_thread;
					get_next_thread(NUH);
					brother->current_thread = struggler->current_thread;
					struggler->current_thread = backup;
					poke_brother(i,OS_INTERRUPTS::HELP_OUT);
					return;
				}
			}
		}
		BLOOD_LIBEL(NUH);
		//get to S1 or something idk
	}
	/*
	Processors taking a rest is done automatically but there is no way to wake processors back up aside from letting IO do it for us so...*/
	void evaluate_workload(void){
		//something something account for the skewing of the threads/ awake processors number and check thermal with acpi
		//if processor is overworked wake another up
		//if we are being overwhelmed BLOOD_LIBEL something

		//something something do a loop where you zero out all of the processsors' executed_threads so that you can tell who is slacking and who got
		//stolen by a driver or something		NOTE this is a crypto-mining anti-feature...
	}
	#define RESCHEDULE LONGJUMP(&routine)
	void routine(void){
		Kontrol * ctrl = get_kontrol_object(NUH);
		Kingmem * mm = get_kingmem_object(NUH);
		Virtual_fs * vfs = get_vfs_object(NUH);
		Processor * processor = get_processor_object(NUH);

		if !(ctrl->devenum_millis > gettime_millis(NUH)){
			reenact_IO(NUH);
		}
		if (ctrl->memory_meridian == 10){
			mm->swap_process(get_process_object(NUH););
			vfs->evictions_cycle(NUH);
		}

		if (executed_threads > ctrl->maxthreads/get_processors_number(void*1)){
			evaluate_workload(NUH);
		}

		getnext_thread(processor);
		++processor->executed_threads;

		run_thread(tking->pool[processor->current_thread);
	}


	/*
	Tomfoolery.
	The interrupt for syscalls is divide by 0 fault
	*/
	struct interrupt_stack{
		uint64_t * errcode,rip,cs,rflags,rsp,ss;
	};
	void os_interrupt_handler_template(constexpr vector){		//there are supposed to be a lot of these in the binary
		ENTER_RINGZERO;
		Thread * thread = get_thread_object(NUH);
		thread->sys_retval = thread->state::accumulator;
		(OS_INTERRUPTS::ROUTINES[vector])(NUH);
		set_task_priority(0);
		leave_ringzero(get_thread_object(NUH));
	}
	void device_interrupt_handler_template(constexpr vector){
		ENTER_RINGZERO;
		Kingmem * mm = get_kingmem_object();
		Virtual_fs * vfs = get_vfs_object(NUH);
		Directory * dev = &vfs->descriptions[1];
		Thread * thread = dev->children->pool[(vector-64)/dev->children->count]->thread;
		thread->prior = get_thread_object(NUH);
		auto * sp = vmto_phys(thread->prior->process->pagetree,thread->prior->state::stack_pointer);
		memcpy(sp,get_stack_pointer(NUH),40);				//copying the iret payload
		set_stack_pointer(sp);
		thread->instruction_pointer = dev->virt_irhandler;
		run_thread(thread);
	}
	void syscall(void){					//gets called by the 0th vector
		SyscallsGod * sgod = get_syscalls_object(NUH);
		Thread * thread = get_thread_object(NUH);
		Process * process = thread->parent;

		enter_ringzero(void);
		ustd_t syscall_number = userspace_stack[0];
		memcpy(userspace_stack-40,ringzero_stack,40);		//endianess
		set_stack_pointer(userspace_stack);			//DANGER ebil magick task linking???
		LONGJUMP(sgod->pool[syscall_number]);
	}
	#define SYSRET{ LONGJUMP(&sysret)}
	void sysret(void){
		Thread * thread = get_thread_object(NUH);
		if (thread->type == thread_types::APPLICATION){
			__non_temporal thread->taken = 0;
			RESCHEDULE;
		}
		if (thread->type == thread_types::KERNEL){
			auto * jmp = thread->prior->state::instruction_pointer+SYSCALL_INSTRUCTION_LENGTH;			//no conversion is needed, NOTE the macro.
			TKILL(thread);
			LONGJUMP(jmp);
		}
		run_thread(thread);
	}
	void timed_out(void){
		ENTER_RINGZERO;
		RESCHEDULE;
	}
	#define BAD_SYSRET(thread,value){thread->sys_retval = value; SYSRET}
	#define CONDITIONAL_SYSRET(thread,condition,value){ if(condition){ thread->sys_retval = value; SYSRET;}}
