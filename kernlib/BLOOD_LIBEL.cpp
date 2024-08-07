/*
The algorithm is: "if not programtype==SYSTEM" memory usage in units of 2mb pages * the number of threads
Whoever has the highest score gets SACRIFICED
*/
void BLOOD_LIBEL(void){
	ProcessesKing * prcking = get_processking_object(void);
	ustd_t i = 0;
	while (prcking->pool[i]->program_type == program_types::SYSTEM);
	Process * champion = &prcking->pool[i];
	if (i > prcking->length){ shutdown(void);}
	for (1; i < prcking->length; ++i){
		Process * contender = &prcking->pool[i];
		if (contender->program_type != program_types::SYSTEM)&&(prcking->ckarray[i]){
			contender->sacrival = contender->memory_usage/512*contender*wk_count;
			if (contender->sacrival > champion->sacrival){
				champion = contender;
			}
		}
	}
	brothers_sleep(void);				//instakilling the process, there is a guarantee that all threads of the SACRIFICE will die before the caller gets executed
	champion->sigset |= signals.SIGKILL;		//*will* cause visible lag but that is a good thing, reboot your machine.
}
