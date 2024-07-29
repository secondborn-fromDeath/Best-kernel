/*
This gets called everywhere around the kernel whenever the filesystem is interacted with through a syscall (or an interrupt, networking)
*/

void report_IO(ustd_t findex, ustd_t action){		//takes an index into the vfs, not a pointer
	ProcessesKing * pking = get_processking_object(void);
	for (ustd_t i = 0; i < pking->length; ++i){
		Process * process = &pking->pool[i];
		for (ustd_t k = 0; k < process->workers->count; ++k){
			Thread * thread = &process->workers->pool[k];
			for (ustd_t w = 0; w < thread->pollnum){
				pollfd * poll = &thread->poll[w];
				if (poll)&&(process->descs[poll->fdesc]->findex == findex){
					poll->retaction = action;
					thread->sigset ^= SIGPOLLING;
				}
			}
		}
	}
}
