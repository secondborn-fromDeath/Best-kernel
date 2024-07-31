/*
Kills the calling thread*/
void SYS_TEXIT(void){
	get_thread_object(void)->sigset |= signals::SIGKILL;
	SYSRET;
}
