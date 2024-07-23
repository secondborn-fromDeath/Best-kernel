void store_state(Thread * thread);
void load_state(Thread * thread);

	void run_ringthree(auto * func){
		schedule_timed_interrupt(SCHEDULER_INTERRUPT_TIMER,EIGHT);	//undefined and random value, see drivers/interrupts.cpp
		uint64_t * sp = get_stack_pointer(void);
		sp[4] = func;
		set_ipi_mode(get_selfipi_mask(void));
		set_task_priority(0);
		iret(void);
	}
	void run_thread(Thread * thread){
		set_thread_object(thread);
		run_ringthree(thread->state->instruction_pointer);
	}
	void getnext_thread(Processor * processor){
		ThreadsKing * tking = get_threadsking_object(void);
		tking->stream_init(void);
		for (ustd_t i = processor->current_thread+1; i != processor->current_thread; ++i){
			if (i == tking->length){ i = 0;}

			if (tking->pool[i]->parent->sigset & signals.SIGKILL){
				--tking->pool[i]->parent->numberof_children;
				if !(tking->pool[i]->parent->numberof_children){
					Processking * processking = get_processking_object(void);
					PKILL(tking->pool[i]->parent,1);	//gets all of our memory back and frees the spot in the process pool
				}
				tking->pool_free(&tking[i],1);		//DANGER not zeroing the thread structures causes problems in the signal handlers
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
		halt(void);		//brother will wake it up.
	}
	//NOTE you likely need functions for the process' signals beyond the kill that is checked above
	ustd_t signal_handler(Thread * thread, ustd_t signal){
		switch (signal){
			case SIGKILL:{	memset(thread,0,sizeof(Thread)); return 1;}
			case SIGTERM:{	if (thread->sigmask & SIGTERM){ run_ringthree(thread->sighandlers[SIGTERM]);} TKILL(thread);}
			case SIGSTOP:{	return 1;}
			case SIGCONT:{	thread->sigset |= SIGSTOP; thread->sigset ^= SIGSTOP; return 0;}
			//TODO		case SIGIO:{	thread->sigset |= SIGPOLLING; thread->sigset ^= SIGPOLLING; return 0;}
			default: {
				if (thread->sighandlers[signals]){
					thread->state->instruction_pointer = thread->sighandlers[signal];
				}
				return 0;
			}
		}
	}
	void check_signals(Thread * thread){
		for (sig_t i = 0; i < MAXSIG){
			if (thread->sigset & i){
				signal_handler(thread,i);
			}
		}
	}
	/*
	Processors taking a rest is done automatically but there is no way to wake processors back up aside from letting IO do it for us so...*/
	void evalutate_workload(void){
		//something something account for the skewing of the threads/processors number with the acpi health methods
		//something something drop the processor's power and let execution be slower as long as all threads get executed
		//if we are being overwhelmed BLOOD_LIBEL something with a lot of threads

		process->executed_threads = 0;
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
			evalluate_workload(void);
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
	void syscall(void){
		set_task_priority(15);
		SyscallsGod * sgod = get_syscalls_object(void);
		Thread * thread = get_thread_object(void);

		interrupt_stack * ringzero_stack = get_stack_pointer(void);
		void * userspace_instruction = ringzero_stack->rip;
		uint64_t * userspace_stack = ringzero_stack->rsp;
		ustd_t syscall_number = userspace_stack*;
		if (thread->type == thread_types.DRIVER){
			memcpy(userspace_stack-32,ringzero_stack,32);		//copying the interrupt stack on the stack that will be used by the syscall
			set_stack_pointer(userspace_stack);			//giving the syscall the userspace stack
			(sgod->pool[syscall_number])(&userspace_stack[1]);	//call, syscall will IRETQ back into the driver code
		}
		else{
			(sgod->pool[syscall_number])(&userspace_stack[1]);	//syscall uses the ringzero stack and will jump to routine
		}
	}
	void sysret(void){
		if (get_thread_object(void)->type == thread_types.DRIVER){
			iret(void);
		}
		else{ routine(void);}
	}
	void timed_out(void){
		set_task_priority(15);
		store_state(get_thread_object(void));
		routine(void);
	}
