extern store_previous_state();
extern get_processor_pointer();

void store_state(void);	//this one needs to be used outside of the scheduler too by the way NOTE
void load_state(Thread * thread);

Processor * get_processor_pointer(void);

Thread * get_next_thread(Processor * processor){
    if (processor->threads[processor->current_thread+1]->tid){
	++processor->current_thread;
	return &processor->threads[processor->current_thread+1];
    }
    return 0;
}
ustd_t signal_handler(Thread * thread, ustd_t signal){
    switch (signal){
	case SIGKILL:{	memset(thread,0,sizeof(Thread)); return 1;}
	case SIGTERM:{	if (thread->sigmask & SIGTERM){POINTER_CALL(thread->sighandlers[SIGTERM]);} return 1;}
	case SIGSTOP:{	return 1;
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
		if (this.signal_handler(i)){
		    return 1;
		}
	    }
	}
	return 0;
    }
    void run_thread(Thread * thread){
	load_state(thread);
	schedule_timed_interrupt();
	__asm__(
	"mov %%rax,%%rsp\n\t"
	"SYSRET\n\t"			//NOTE not too sure where rip is on the stack kek
	::"r"(thread->state->instruction_pointer):
	);
    }


    /*
     *		There are a lot of problems here, for one "evaluate memory", for two a lot of functions are abstrcated out, for three "routine()" does not return
     */
    void routine(void){
	store_state(void);
	Kingmem * mm = get_kingmem_object(void);
	Processor * processor = get_processor_pointer(void);
	Thread * next_thread;
	ustd_t dont_mess_with_pm = 0;
	for (ustd_t i = 0; 1; ++i){
	    if (i == processor->max_threads){
		if !(dont_mess_with_pm)	Ã++dont_mess_with_pm; //TODO go low power, this is nonsense though, you probably just halt and have another processor look at the number of threads and be like "what the fuck bros" and wake the others up
	    }

	    next_thread = get_next_thread(processor);
	    if !(next_thread){
		processor->current_thread = 0;
	    }
	    else if !(check_signals(thread)){
		mm->evaluate_memory(next_thread->parent);
		run_thread(next_thread);
	    }
	}
    }
