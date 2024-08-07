/*
Control register reference because it is needed:Ã
11:	thread object pointer
6:	data pointer
13:	ringzero pagetree
5:	information pass, boot
9:	information count, boot
15:	used by bsp for the APid in serve_data()
12,14:	store_state()
1:	stack backup on runtime halts
7:	task priority on runtime halts
*/

/*
Application processor portion of things*/
void prepare_controls(void){
	__asm__ volatile(
	"XORL	%%eax,%%eax\n\t"
	"MOVQ	%%rax,%%cr6\n\t"
	"MOVQ	%%rax,%%cr9\n\t"
	"JMP +0\n\t"
	:::);
	halt(NUH);
}
void sipi(void){
	processor_init(NUH);	//does cli() internally
	sti(NUH);
	prepare_controls(NUH);
	halt(NUH);
}
void templ_receive_info(constexpr ustd_t num){			//this is hooked with constexpr to the low 16 operating system interrupts
	ustd_t a;
	__asm__ volatile(
	"MOVQ	%%cr5,%%rcx\n\t"	//data
	"MOVQ	%%cr9,%%rax\n\t"	//bit offset
	"MOVQ	I0,%%rdx\n\t"		//loading relative interrupt vector
	"SHLQ	%%rax,%%rdx\n\t"	//shifting and accumulating
	"ORQ	%%rdx,%%rcx\n\t"
	"MOVQ	%%rcx,%%cr5\n\t"	//storing data
	"JMP	+0\n\t"			//pipeline serialization
	"ADDQ	$4,%%rax\n\t"		//adding to the bitshift
	"MOVQ	%%rax,%%cr9\n\t"	//...
	"JMP	+0\n\t"
	::"r"(a),"I"(num):);
	if (a == 64){
		request_data(NUH);
	}
	else{ halt(NUH);}
}
void request_data(void){
	ustd_t bsp_id;
	__asm__ volatile(
	"MOVQ	%%cr5,%%rax\n\t"
	::"r"(bsp_id):);

	poke_brother(bsp_id,OS_INTERRUPTS::REQUEST_DATA_POINTER);
	halt(NUH);
}
void receive_data(void){
	ustd_t bsp_id;
	__asm__ volatile(
	"MOVQ	%%cr5,%%rax\n\t"
	::"r"(bsp_id):);

	void * data = remote_read(bsp_id,0x280);

	__asm__ volatile(
	"MOVQ	%%rax,%%cr6\n\t"
	"JMP	+0\n\t"
	::"r"(data):);

	//setting the ringzero pagetree
	__asm__ volatile(
	"MOVQ	%%rax,%%cr13\n\t"
	"JMP	+0\n\t"
	::"r"(get_kingmem_object(NUH)->vm_ram_table):);

	poke_brother(bsp_id,OS_INTERRUPTS::ENDOF_TRANSACTION);
	halt(NUH);
}




/*
This is the BSP portion of things*/
void serve_data(void){
	Kontrol * ctrl = get_kontrol_object(NUH);
	ustd_t local_apic = ctrl->lapic_override;

	local_apic[0x280/4] = get_kontrol_object(NUH);				//DANGER dont change things around in kernlib
	ustd_t APid;
	__asm__ volatile("MOVQ	%%cr15,%%rax\n\t" ::"r"(APid):);
	brothers_poke(APid,OS_INTERRUPTS::NOTIFY_DATA);
	halt(NUH);
}
//THE ROUTINE:
void initialize_brothers(void){
	ushort_t bsp_id = get_processor_id(NUH);
	bsp_id = rotateRight((char)(bsp_id<<4),4);		//now the low 4 bits are 0-3, the high 4 are 8-11...

	ustd_t mask = get_startupmode_mask(NUH) | OS_INTERRUPTS::SLEEP;	//doing some weird battery bus shit
	brothers_poke(mask);

	for (ustd_t i = 0; i < bsp_id; ++i){
		__asm__ volatile("MOVQ	%%rax,%%cr15\n\t" ::"r"(i):);
		poke_brother(i,OS_INTERRUPTS::FIRST_SIPI);
		schedule_timed_interrupt(4096,1);		//waiting for a bunch of instructions before sending the other signals
		poke_brother(i,32+((char)bsp_id));
		schedule_timed_interrupt(4096,1);
		rotateRight(bsp_id,8);
		poke_brother(i,32+((char)bsp_id));
		halt(NUH);					//NOTE might want to stay awake and read error codes
		rotateLeft(bsp_id,8);
	}
}
