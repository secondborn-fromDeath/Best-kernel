extern get_processor_pointer(void);

void store_state(Thread * thread);
void load_state(Thread * thread);

	void run_ringthree(auto * func){
		schedule_timed_interrupt(SCHEDULER_INTERRUPT_TIMER,EIGHT);	//undefined and random value, see drivers/interrupts.cpp
		__asm__(
		"MOV	%%rax,32(%%rsp)\n\t"	//exhanging the return address with func
		"IRETQ\n\t"
		::"r"(func):);
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
					processking->pool_free(tking->pool[i]->parent,1);
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
		__asm__("HLT");		//brother will wake it up.
	}

	ustd_t signal_handler(Thread * thread, ustd_t signal){
		switch (signal){
			case SIGKILL:{	memset(thread,0,sizeof(Thread)); return 1;}
			case SIGTERM:{	if (thread->sigmask & SIGTERM){ run_ringthree(thread->sighandlers[SIGTERM]);} return 1;}
			case SIGSTOP:{	return 1;}
			case SIGCONT:{	thread->sigset |= SIGSTOP; thread->sigset ^= SIGSTOP; return 0;}
			case SIGOFLOW:{	if (thread->sigmask & SIGOFLOW){ run_ringthree(thread->sighandlers[SIGOFLOW]);} return 0;}
			case SIGUFLOW:{	if (thread->sigmask & SIGUFLOW){ run_ringthree(thread->sighandlers[SIGUFLOW]);} return 0;}
			case SIGFPE:{	if (thread->sigmask & SIGFPE){ run_ringthree(thread->sighandlers[SIGFPE]);} return 0;}
			//TODO		case SIGIO:{	thread->sigset |= SIGPOLLING; thread->sigset ^= SIGPOLLING; return 0;}
			default: {
				if (thread->sighandlers[signals]){
					run_ringthree(thread->sighandlers[signal]));
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
	void routine(void){
		State prior;
		store_state(prior);	//saving state on the userspace stack

		Kontrol * ctrl = get_kontrol_object(void);
		Kingmem * mm = get_kingmem_object(void);
		Virtual_fs * vfs = get_vfs_object(void);
		Processor * processor = get_processor_object(void);

		if (ctrl->memory_meridian == 10){
			mm->swap_process(get_process_object(void););
			vfs->evictions_cycle(void);
		}

		do{
			getnext_thread(processor);
		}while(processor->current_thread != -1);
		++processor->executed_threads;

		load_state(get_current_thread(void));	//NOTE this the get_process_object MUST NOT be merged
		run_ringthree(tking->pool[processor->current_thread]->state->instruction_pointer);
	}
	/*
	Because i dont have compiler-aid the arguments are passed on the userspace stack, reminder of the C calling convention for acd*/
	void syscall(void){
		store_state(get_thread_object(void));
		__asm__ volatile(
		"MOVq	8(%%rsp),%%rdx\n\t"			//reminder, from the bottom up: ss,rsp,rflags,cs,rip
		"MOVq	(%%rdx),%%rcx\n\t"			//getting syscall number from userspace stack
		"MOVq	(%%rax,%%rcx),%%rax\n\t"		//function pointer into rax
		"ADDq	$8,%%rdx\n\t"				//passing the userspace stack as argument
		"PUSHq	%%rdx\n\t"
		"CALL	%%rax"					//calling the function pointer from ringzero
		::"r"((get_syscallsgod_object)->a):);

		routine(void);
	}
	void timed_out(void){
		store_state(get_thread_object(void));
		routine(void);
	}
