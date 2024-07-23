auto * get_stack_pointer(void){
	auto * ret;
	__asm__("MOVq	%%rsp,%%rax\n\t" ::"r"(ret):);
}
void set_stack_pointer(auto * newstack){
	__asm__("MOVq	%%rax,%%rsp\n\t" ::"r"(newstack):);	//should be fine...
}
