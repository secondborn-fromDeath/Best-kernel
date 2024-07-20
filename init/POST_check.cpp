//reading from the keyboard controller configuration register wether the POST check was performed

void POST_check(void){
	uint8_t a;
	__asm__(
	"mov $0x20,%%al\n\t"	//cmd: Read first byte of internal RAM (which is the configuration byte)
	"OUT %%al,$0x64\n\t"
	"IN $0x60,%%al\n\t"
	::"r"(a):);

	if !(a & 4){
		Kontrol * ctrl = get_kontrol_object(void);
		__asm__(
		"MOV	%%eax,%%edx\n\t"
		"OUT	%%ax,%%dx\n\t"
		::"r"(ctrl->shutdown_port));
	}
}
