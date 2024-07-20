//NOTE reminder to compile with -m16


extern enable_A20(void);
extern mswitch_16to32(void);


void cli(void){ __asm__("CLI\n\t");
}
void secure_stackstate(void){				//standardizing the stack between IA32 and x64, gcc should cause no problems
	__asm__ volatile(
	"TEST %%ecx,%%ecx\n\t"
	"JZ skip_pushes_efistate\n\t"
	"MOVl (%%esp),%%eax\n\t"
	"MOVl %%ecx,(%%esp)\n\t"			//efi image handle
	"PUSH %%edx\n\t"				//efi system table
	"PUSH %%eax\n\t"				//moving back the return address and returning
	"skip_pushes_efistate:\n\t"
	:::);
}
void UefiMain(void * imagehandle, void * efisystab){
	cli(void);
	secure_stackstate(void);

	enable_A20(void);
	setup_gdt(void);
	mswitch_16to32(void);
	auto * func = &32bit_start;
	__asm__("JMPF %0\n\t"
		::"r"(func):);		//NOTE danger
}
