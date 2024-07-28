/*
Control register reference because it is needed:Ã
11:	thread object pointer
6:	data pointer
13:	ringzero pagetree
5:	information pass, boot
9:	information count, boot
15:	used by bsp for the APid in serve_data()*/




/*
Application processor portion of things*/
void prepare_controls(void){
	__asm__ volatile(
	"XORl	%%eax,%%eax\n\t"
	"MOVq	%%rax,%%cr6\n\t"
	"MOVq	%%rax,%%Cr9\n\t"
	"JMP +0\n\t"
	:::);
	halt(void);
}
void first_sipi(void){
	processor_init(void);	//does cli() internally
	sti(void);
	prepare_controls(void);
	halt(void);
}
void request_data(void){
	ustd_t bsp_id;
	__asm__ volatile(
	"MOVq	%%cr5,%%rax\n\t"
	::"r"(bsp_id):);

	poke_brother(bsp_id,OS_INTERRUPTS::REQUEST_DATA_POINTER);
	halt(void);
}
void receive_data(void){
	ustd_t bsp_id;
	__asm__ volatile(
	"MOVq	%%cr5,%%rax\n\t"
	::"r"(bsp_id):);

	void * data = remote_read(bsp_id,0x280);

	__asm__ volatile(
	"MOVq	%%rax,%%cr6\n\t"
	"JMP	+0\n\t"
	::"r"(data):);

	//setting the ringzero pagetree
	__asm__ volatile(
	"MOVq	%%rax,%%cr13\n\t"
	"JMP	+0\n\t"
	::"r"(get_kingmem_object(void)->vm_ram_table):);

	poke_brother(bsp_id,OS_INTERRUPTS::ENDOF_TRANSACTION);
	halt(void);
}
void templ_receive_info(constexpr ustd_t num){			//this is hooked with constexpr to the low 16 operating system interrupts
	ustd_t a;
	__asm__ volatile(
	"MOVq	%%cr5,%%rcx\n\t"	//data
	"MOVq	%%cr9,%%rax\n\t"	//bit offset
	"MOVq	I0,%%rdx\n\t"		//loading relative interrupt vector
	"SHLq	%%rax,%%rdx\n\t"	//shifting and accumulating
	"ORq	%%rdx,%%rcx\n\t"
	"MOVq	%%rcx,%%cr5\n\t"	//storing data
	"JMP	+0\n\t"			//pipeline serialization
	"ADDq	$4,%%rax\n\t"		//adding to the bitshift
	"MOVq	%%rax,%%cr9\n\t"	//...
	"JMP	+0\n\t"
	::"r"(a),"I"(num):);
	if (a == 64){
		request_data(void);
	}
	else{ halt(void);}
}




/*
This is the BSP portion of things*/
void serve_data(void){
	Kontrol * ctrl = get_kontrol_object(void);
	ustd_t local_apic = ctrl->lapic_override;

	local_apic[0x280/4] = get_kontrol_object(void);				//DANGER dont change things around in kernlib
	ustd_t APid;
	__asm__ volatile("MOVq	%%cr15,%%rax\n\t" ::"r"(APid):);
	poke_brother(APid,OS_INTERRUPTS::NOTIFY_DATA);
	halt(void);
}
//THE ROUTINE:
void initialize_brothers(void){
	ushort_t bsp_id = get_processor_id(void);
	bsp_id = rotateRight((char)(bsp_id<<4),4);		//now the low 4 bits are 0-3, the high 4 are 8-11...

	for (ustd_t i = 0; i < bsp_id; ++i){
		__asm__ volatile("MOVq	%%rax,%%cr15\n\t" ::"r"(i):);
		poke_brother(i,OS_INTERRUPTS::FIRST_SIPI);
		schedule_timed_interrupt(4096,1);		//waiting for a bunch of instructions before sending the other signals
		poke_brother(i,32+((char)bsp_id));
		schedule_timed_interrupt(4096,1);
		rotateRight(bsp_id,8);
		poke_brother(i,32+((char)bsp_id));
		halt(void);				//NOTE might want to stay awake and read error codes
		rotateLeft(bsp_id,8);
	}
}
