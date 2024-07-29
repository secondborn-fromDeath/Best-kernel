#define LONGJUMP(x){ __asm__ volatile("JMP	%%rax\n\t" ::"r"(x):);}
