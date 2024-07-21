void secure_stackstate(void){			//standardizing the stack between IA32 and x64, gcc>
	__asm__ volatile(
	"TEST %%ecx,%%ecx\n\t"
	"JZ skip_pushes_efistate\n\t"
	"MOVl (%%esp),%%eax\n\t"
	"MOVl %%ecx,(%%esp)\n\t"		//efi image handle
	"PUSH %%edx\n\t"			//efi system table
	"PUSH %%eax\n\t"			//moving back the return address and returning
	"skip_pushes_efistate:\n\t"
	:::);
}
