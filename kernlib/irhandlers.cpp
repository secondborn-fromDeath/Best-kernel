/*
See runtime/sched/sched.cpp for the template interrupt routine
*/




void killer(void){
	Process * process = get_process_object(NUH);
	process->sigset = signals::SIGKILL;
}
void signal_issuer(ustd_t signal){
	Thread * thread = get_thread_object(NUH);
	auto * insert = thread->sighandler[constexpr signalto_index(signal)];
	auto ** pointer = &thread->state.instruction_override;
	for (ustd_t i = 0; i < 16; ++i){
		if (pointer[i] == 0){
			pointer[i] = insert;}
	}
	if (i == 16){
		thread->parent->sigset |= signals::SIGKILL;		//the only other way is to make signals a bigger information packet, on which idk what you would do
	}
}
void breakpoint_(void){
	signal_issuer(signals::BREAKPOINT);
}
void fpe(void){
	signal_issuer(signals::SIGFPE);
}
void simd_fpe(void){
	signal_issuer(signals::SIGSIMD);
}
void signal_handler_return(void){
	Thread * thread = get_thread_object(NUH);
	auto ** codes = &thread->state.instruction_override;
	for (ustd_t i = 0; i < 15; ++i){
		codes[i] = codes[i+1];
	}
	SYSRET;
}
void acpi_machine_check(void){ shutdown(void);}				//i havent done acpi to that degree yet.
void double_fault_handler(void){ shutdown(void);}
