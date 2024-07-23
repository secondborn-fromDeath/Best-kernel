void * get_kernelcode_pointer(void){
	__asm__ volatile(
	"MOV	-KERNELCODE_START(%%rip),%%rax\n\t"
	"RET\n\t"
	);
}
