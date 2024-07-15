extern store_previous_state();
extern get_processor_pointer();

void store_state(void);	//this one needs to be used outside of the scheduler too by the way NOTE
void load_state(Thread * thread);

	Processor * get_processor_pointer(void);

	void getnext_thread(Processor * processor){
		ThreadsKing * tking; get_threadsking_object(tking);
		ustd_t processor_id = tking->stream_init(void);
		for (ustd_t i = processor->current_thread+1; i != processor->current_thread; ++i){
			if (i == tking->max_threads){ i = 0;}

			if (tking->pool[i]->parent->sigset & signals.SIGKILL){
				--tking->pool[i]->parent->children;
				if !(tking->pool[i]->parent->children){
					memset(tking->pool[i]->parent,0,sizeof(Process));
				}
				memset(&tking->pool[i],0,sizeof(Thread));
			}

			if (tking->pool[i]->tid != 0){
				processor->current_thread = i;
				__non_temporal tking->calendar[processor_id] = 0;
				return;
			}
		}
		__non_temporal tking->calendar[processor_id] = 0;
		processor->current_thread = -1;
		__asm__("HLT");	//will return after the interrupt routine dont worry
	}

	ustd_t signal_handler(Thread * thread, ustd_t signal){
		switch (signal){
			case SIGKILL:{	memset(thread,0,sizeof(Thread)); return 1;}
			case SIGTERM:{	if (thread->sigmask & SIGTERM){POINTER_CALL(thread->sighandlers[SIGTERM]);} return 1;}
			case SIGSTOP:{	return 1;}
			case SIGCONT:{	thread->sigset |= SIGSTOP; thread->sigset ^= SIGSTOP; return 0;}
			case SIGOFLOW:{	if (thread->sigmask & SIGOFLOW){POINTER_CALL(thread->sighandlers[SIGOFLOW]);} return 0;}
			case SIGUFLOW:{	if (thread->sigmask & SIGUFLOW){POINTER_CALL(thread->sighandlers[SIGUFLOW]);} return 0;}
			case SIGFPE:{	if (thread->sigmask & SIGFPE){POINTER_CALL(thread->sighandlers[SIGFPE]);} return 0;}
			//TODO		case SIGIO:{	thread->sigset |= SIGPOLLING; thread->sigset ^= SIGPOLLING; return 0;}
			case SIGPOLLING:{ return 1;}
			case SIGUSR0:{	if (thread->sigmask & SIGUSR0){POINTER_CALL(thread->sighandlers[SIGUSR0]);} return 0;}
			case SIGUSR1:{	if (thread->sigmask & SIGUSR1){POINTER_CALL(thread->sighandlers[SIGUSR1]);} return 0;}
			case SIGUSR2:{	if (thread->sigmask & SIGUSR2){POINTER_CALL(thread->sighandlers[SIGUSR2]);} return 0;}
			case SIGUSR3:{	if (thread->sigmask & SIGUSR3){POINTER_CALL(thread->sighandlers[SIGUSR3]);} return 0;}
			case SIGUSR4:{	if (thread->sigmask & SIGUSR4){POINTER_CALL(thread->sighandlers[SIGUSR4]);} return 0;}
			case SIGUSR5:{	if (thread->sigmask & SIGUSR5){POINTER_CALL(thread->sighandlers[SIGUSR5]);} return 0;}
			//default:{	//TODO print bug info or something idk, this shouldnt be happening
		}
	}
	std_t check_signals(Thread * thread){
		for (sig_t i = 0; i < MAXSIG){
			if (thread->sigset & i){
				if (this.signal_handler(i)){	//shouldnt this one instead be "if the handler is there execute it"???? strange
					return 1;
				}
			}
		}
		return 0;
	}

	void routine(void){
		store_state(void);

		Kontrol * ctrl; get_kontrol_object(ctrl);
		Kingmem * mm; get_kingmem_object(mm);
		Virtual_fs * vfs; get_vfs_object(vfs);
		Processor * processor = get_processor_structure(void);

		if (ctrl->memory_meridian == 10){
			mm->swap_process(get_process_object(void););
			vfs->evictions_cycle(void);
		}

		do{
			getnext_thread(processor);
		}while(processor->current_thread != -1);
		schedload_state(tking->pool[processor->current_thread]);	//this one puts the new instruction pointer and stack on the stack

		schedule_timer_interrupt(SCHEDULER_INTERRUPT_TIMING,EIGHT);	//random values really
		__asm__ ("SYSRET\n\t");
	}
