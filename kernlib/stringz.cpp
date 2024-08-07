//this is a function to set the direction flag
void std(void){ __asm__ volatile("STDn\t");}
//the opposite
void cld(void){ __asm__ volatile("CLDn\t");}


/*
This is a loop that keeps on going until thee byte is not zero, returns the pointer to the nonzero instance
8byte alignment is a requirement*/
void * stringz(void * start){
__asm__ volatile(
"MOVQ	%%rax,%%rsi\n\t"
"XORL	%%eax,%%eax\n\t"
"REPZ	SCASQ\n\t"
"MOVQ	%%rsi,%%rax\n\t"
:"=r"(start):"r"(start):"%rsi","%rdi")
return start;
}
