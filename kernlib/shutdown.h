void shutdown(void){
	Kontrol * ctrl = get_kontrol_object(void);
	__asm__(
	"MOVq	%%rax,%%rdx\n\t"			//read the spec idk idc
	"OUTw	%%ax,%%dx\n\t"
	::"r"(ctrl->shutdown_port):);

}
