using namespace signals;

void store_state(Thread * thread);	//reminder these do gdt,ldt and pagetree... possibly in the future tss???? NOTE OPTIMIZATION
void load_state(Thread * thread);
void load_stack(Thread * thread);		//unprivileged sp,ss,cd,rip... i will figure out what do about flags

	__attribute__((interrupt)) void run_ringthree(Thread * thread){
		set_ipi_mode(get_selfipi_mask(void));
		set_task_priority(0);
		load_state(thread);
		load_stack(thread);
		if (thread->type == APPLICATION) {schedule_timed_interrupt(SCHEDULER_INTERRUPT_TIMER,EIGHT);}	//undefined and random value, see drivers/interrupts.cpp
	}
	void run_thread(Thread * thread){
		set_thread_object(thread);
		run_ringthree(thread);
	}
	void help_out(void){
		Processor * processor = get_processor_object(void);
		run_thread(&get_threadsking_object(void)->pool[processor->current_thread]);
	}
	void getnext_thread(Processor * processor){
		ThreadsKing * tking = get_threadsking_object(void);
		tking->stream_init(void);
		for (ustd_t i = processor->current_thread+1; i != processor->current_thread; ++i){
			if (i == tking->length){ i = 0;}

			if (tking->pool[i]->parent->sigset & SIGKILL){
				--tking->pool[i]->parent->numberof_children;
				if !(tking->pool[i]->parent->numberof_children){
					Processking * processking = get_processking_object(void);
					PKILL(tking->pool[i]->parent,1);	//gets all of our memory back and frees the spot in the process pool, the memset should be in there
				}
				TKILL(thread);
			}

			if (tking->pool[i]->type == threadtypes.DRIVER){ tking->pool_free(&tking[i],1);}	//see modinsert
			if !(tking->pool[i]->taken){
				processor->current_thread = i;
				__non_temporal tking->calendar = 0;
				return;
			}
		}
		__non_temporal tking->calendar = 0;
		processor->current_thread = -1;
		halt(void);
	}
	//NOTE you likely need functions for the process' signals beyond the kill that is checked above			yeah but does this check the fucking thread or the process? fuck
	ustd_t signal_handler(Thread * thread, ustd_t signal){
		switch (signal){
			case SIGKILL:{	TKILL(thread); return 1;}
			case SIGSTOP:{	return 1;}
			case SIGCONT:{	thread->sigset |= SIGSTOP; thread->sigset ^= SIGSTOP; return 0;}
			default: {
				if (thread->sighandlers[signals]){
					thread->state->instruction_pointer = thread->sighandlers[signal];	//this includes sigterm
				}
				return 0;
			}
		}
	}
	/*
	Checks both the thread local signals and those of the process*/
	void check_signals(Thread * thread){
		for (sig_t i = 0; i < MAXSIG; +i){
			if (thread->sigset & i){
				signal_handler(thread,i);
			}
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
		ProcessorsGod * processors = get_processorsgod_object(void);
		for (ustd_t i = 0; i < get_processors_number(void); ++i){
			Processor * brother = &processors->pool[i];
			if !(brother->executed_threads){
				if (brother->online_capable){
					Processor * struggler = get_processor_object(void);
					ustd_t backup = struggler->current_thread;
					get_next_thread(void);					//NOTE MAKE SURE THERE ARE FREE THREADS
					brother->current_thread = struggler->current_thread;
					struggler->current_thread = backup;
					poke_brother(i,OS_INTERRUPTS::HELP_OUT);
					return;
				}
			}
		}
		BLOOD_LIBEL(void);
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
	void routine(void){
		Kontrol * ctrl = get_kontrol_object(void);
		Kingmem * mm = get_kingmem_object(void);
		Virtual_fs * vfs = get_vfs_object(void);
		Processor * processor = get_processor_object(void);

		if (ctrl->memory_meridian == 10){
			mm->swap_process(get_process_object(void););
			vfs->evictions_cycle(void);
		}

		if (executed_threads > ctrl->maxthreads/get_processors_number(void*1)){
			evaluate_workload(void);
		}

		getnext_thread(processor);
		++processor->executed_threads;

		run_thread(tking->pool[processor->current_thread);
	}


	void enter_ringzero(void){
		set_task_priority(15);
		store_state(get_thread_object(void));
		set_gdt(get_kontrol_object(void)->gdt);
	}
	/*
	Tomfoolery.
	The interrupt for syscalls is divide by 0 fault
	*/
	struct interrupt_stack{
		uint64_t * errcode,rip,cs,rflags,rsp,ss;
	};
	__attribute__((interrupt)) void interrupt_handler_template(ustd_t num){		//there are supposed to be a lot of these in the binary
		enter_ringzero(void);
		(OS_INTERRUPTS::ROUTINES[num])(void);
		set_task_priority(0);
		load_state(get_thread_object(void));
	}
	void syscall(void){					//gets called by the 0th of above
		SyscallsGod * sgod = get_syscalls_object(void);

		interrupt_stack * ringzero_stack = (get_stack_pointer(void) -24)/16*16;	//rounding for the function calls...
		void * userspace_instruction = ringzero_stack->rip;
		uint64_t * userspace_stack = ringzero_stack->rsp;
		ustd_t syscall_number = userspace_stack[0];
		memcpy(userspace_stack-40,ringzero_stack,40);
		set_stack_pointer(userspace_stack);			//DANGER task linking
		LONGJUMP(sgod->pool[syscall_number]);			//if type==DRIVER syscall returns with iret into the same stack, otherwise jumps to routine
	}
	#define SYSRET{ LONGJUMP(&sysret)}
	void sysret(void){
		Thread * thread = get_thread_object(void);
		thread->taken = 0;
		if (get_thread_object(void)->type == thread_types.APPLICATION){
			LONGJUMP(&routine);
		}
	}
	void timed_out(void){
		enter_ringzero(void);
		routine(void);
	}
