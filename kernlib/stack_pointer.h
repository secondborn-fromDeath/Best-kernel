auto * get_stack_pointer(void){
	auto * ret;
	__asm__ volatile("MOVq	%%rsp,%%rax\n\t" :"=r"(ret)::);
}
void set_stack_pointer(auto * newstack){
	__asm__ volatile("MOVq	%%rax,%%rsp\n\t" ::"r"(newstack):);	//should be fine...
}
