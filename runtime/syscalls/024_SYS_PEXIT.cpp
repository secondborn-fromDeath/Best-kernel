/*
Kills the calling process*/
void SYS_PEXIT(void){
	get_process_object(void)->sigset |= SIGKILL;
	SYSRET;
}
