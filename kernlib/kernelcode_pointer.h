void * get_kernelcode_pointer(void){
	void * ret;
	__asm__ volatile(
	"CALL	+0\n\t"
	"POPQ	%%rax\n\t"
	"ADDQ	_start,%%rax\n\t"	//i think the math is right? the label should be negative
	:"=r"(ret)::);
	return ret;
}
ulong_t get_kernelcode_size(void){
	ulong_t ret = (ulong_t)get_kernelcode_pointer(void);
	ulong_t whore;
	__asm__ volatile{
	"CALL	+0\n\t"
	"POPQ	%%rax\n\t"
	"ADDQ	_end_of_code,%%rax\n\t"
	"SUBQ	%%rax,%%rcx\n\t"
	"MOVQ	%%rcx,%%rax\n\t"
	:"=r"(ret):"r"(whore),"r"(ret):}
	return ret;
}
