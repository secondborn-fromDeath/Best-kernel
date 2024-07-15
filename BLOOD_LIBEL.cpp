/*
This one evaluates all processes and gives them a BLOOD_score which is made up of their memory usage, owner id and programtype

the algorithm is: memory usage in units of 2mb pages, owner id is multiplied by 50, programtype is multiplied by 100, addition
Whoever has the highest score gets BOOTED
*/
void BLOOD_LIBEL(void){
	ProcessesKing * prcking; get_processking_object(prcking);
	ustd_t sacrifice_index = 0;
	for (ustd_t i = 0; i < prcking->length; ++i){
		Process * newsub = prcking->pool[i];
		Process * champion = prcking->pool[sacrifice_index];

		if !(newsub->memory_usage/512+owner_id*50+program_type*100 < champion->memory_usage/512+champion->owner_id*50+champion->program_type*100){
			sacrifice_index = i;
		}
	}
	prcking->pool[sacrifice_index]->sigset |= signals.SIGKILL;
}
