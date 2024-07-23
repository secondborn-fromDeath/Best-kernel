/*
Settign up all of the Model Specific Registers

in order:
//	MTRRS		using the firmware's configuration
	PAT
	TR
//	STARS (syscalls)	i do manual self-ipi interrupts
	various controls
	rflags (serial port privilege)
*/

// this is the enum from runtime/mem/core.h		intel manual:	page 3455 for memory type enum, 4554 for the PAT MSR (address 0x277)
//enum cache{ STRONGUNCACHEABLE,WRITETHROUGH,WRITEBACK,WEAKUNCACHEABLE,WRITEPROTECT,WRITECOMBINING,};
void setup_PAT(void){
	ulong_t c = 0x277;
	ulong_t mask = (0<<0)|(1<<24)|(2<<40)|(3<<8)|(4<<16)|(5<<32);	//the rule is 3 for the type, 5 reserved
	wrmsr(x,mask<<32>>32,mask>>32);
}
void setup_TR(void){
	ustd_t g = get_processor_id(void) + 256+1;
	__asm__ volatile(
	"LTR %%ax\n\t"
	::"r"(g):);
}
void setup_CTRLS(void){			//too much to explain step by step, the page is 3073
	ustd_t zero = (1<<31)|1;
	ustd_t three = 0;
	ustd_t four = (1<<5)|(1<<6)|(1<<8)|(1<<9)|(1<<10)|(1<<16)|(1<18)|(1<<19)|(1<<25);
	ustd_t eight = 0;		//i do this manually with the local apic so its whatever...

	__asm__(
	"MOVl	%%eax,%%cr0\n\t"
	"JMP\n\t"
	"MOVl	%%ecx,%%cr3\n\t"
	"JMP\n\t"
	"MOVl	%%edx,%%cr4\n\t"
	"JMP\n\t"
	"MOVl	%%ebx,%%cr8\n\t"
	"JMP\n\t"
	::"r"(zero),"r"(three),"r"(four),"r"(eight):)
}
void setup_FLAGS(void){	//page 3070		NOTE this is done, interrupts are masked
}
void setup_MSRS(void){
	setup_PAT(void);
	setup_TR(void);
	setup_CTRLS(void);
	setup_FLAGS(void);
}

