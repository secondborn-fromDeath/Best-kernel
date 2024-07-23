#define wrmsr(c,a,d){
	__asm__ volatile(
	"WRMSR\n\t"
	::"r"(a),"r"(c),"r"(d):);	//c is register selector, a is bottom, d is top
}
#define rdmsr(c,a,d){
	__asm__ volatile(
	"WRMSR\n\t"
	::"r"(a),"r"(c),"r"(d):);	//c is register selector, a is bottom, d is top
}
