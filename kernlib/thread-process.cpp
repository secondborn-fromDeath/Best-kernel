volatile Thread * get_thread_object(void){
	Thread * ret;
	__asm__ volatile(
	"MOVq	%%cr11,%%rax\n\t"
	:"=r"(ret)::);
	return ret;volatile Thread * get_thread_object(void){
	Thread * ret;
	__asm__ volatile(
	"MOVq	%%cr11,%%rax\n\t"
	:"=r"(ret)::);
	return ret;
}
void set_thread_object(Thread * pointer){
	__asm__ volatile(
	"MOVq	%%cr11,%%rax\n\t"
	::"r"(pointer):);
}
Process * get_calling_process(void){
	Thread * thread = get_thread_object(NUH);
	return thread->parent;
}

}
void set_thread_object(Thread * pointer){
	__asm__ volatile(
	"MOVq	%%cr11,%%rax\n\t"
	::"r"(pointer):);
}
Process * get_calling_process(void){
	return get_thread_object(void)->parent;
}
