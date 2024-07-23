Thread * get_thread_object(void){
	Thread * ret;
	__asm__(
	"MOVq	%%cr11,%%rax\n\t"
	::"r"(ret):);
	return ret;
}
void set_thread_object(Thread * pointer){
	__asm__(
	"MOVq	%%cr11,%%rax\n\t"
	::"r"(pointer):);
}
Process * get_calling_process(void){
	return get_thread_object(void)->parent;
}
