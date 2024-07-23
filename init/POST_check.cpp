//reading from the keyboard controller configuration register wether the POST check was performed

void POST_check(void){
	uint8_t a;
	__asm__(
	"mov $0x20,%%al\n\t"	//cmd: Read first byte of internal RAM (which is the configuration byte)
	"OUT %%al,$0x64\n\t"
	"IN $0x60,%%al\n\t"
	::"r"(a):);

	if !(a & 4){
		shutdown(void);
	}
}
