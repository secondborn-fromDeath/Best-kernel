//this is a function to set the direction flag
void std(void){ __asm__ volatile("STDn\t");}
//the opposite
void std(void){ __asm__ volatile("CLDn\t");}


/*
This is a loop that keeps on going until thee byte is not zero, returns the pointer to the nonzero instance
8byte alignment is a requirement*/
void * stringz(void * start){
__asm__ volatile(
"MOVq	%%rax,%%rsi\n\t"
"XORl	%%eax,%%eax\n\t"
"REPz	SCASQ\n\t"
"MOVq	%%rsi,%%rax\n\t"
::"r"(start):"%rsi","%rdi")
return start;
}
