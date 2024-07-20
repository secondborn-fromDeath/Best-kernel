extern get_processors_number(void);
extern constexpr auto * interrupt_routines[];


void * get_gdt_offset(void);
void set_idt(void * idt);
void set_gdt(void * gdt, ustd_t size);




namespace gate_types{
#define 0xE INTERRUPT
#define 0xF EXECPTION
};
void Idescriptor_insert(uint64_t * ins, uint64_t * target, ustd_t gdt_segsel, ustd_t tss_entry, ustd_t gate_type, ustd_t privilege_level){
	target[0] = ((ins*)<<48>>48)|((ins*)>>16<<48)|gdt_segsel<<16|tss_entry<<32|gate_type<<40|privilege_level<<45|<<47;		//no need for comments
	target[1] = ((ins*))>>32<<32;													//top of the pointer
}
void setup_idt(void){
	void * idt = uefi_poolalloc(256*16);
	memcpy(idt,get_idt_offset(void),32*16);	//these are probably 8 bytes in reality (protecc firmware) but whatever
	uint64_t * target = idt;

	uint64_t * kernelcode = get_kernelcode_base(void);

	using namespace gate_types;
	ustd_t h = 0;
	ustd_t interrupt_stack = 1;
	ustd_t gate_type = EXCEPTION;
	constexpr for (ustd_t i = 0; i < 7*16; ++i){	//i dont use the top vectors
		constexpr if (interrupt_routines[i]){
			Idescriptor_insert(interrupt_routines[i]+kernelcode,target,i,interrupt_stack,gate_type,0);	//all ring zero, i use SYSCALL
			target += 16;
		}
		if (h == 16){ ++interrupt_stack; h = 0;}
		if (interrupt_stack == 2){ gate_type = INTERRUPT;}
	}
	constexpr for (ustd_t i = 7*16; i < 256; ++i){	//i dont use the top vectors
		target* = NULL;				//present bit is not set...
		target += 8;
	}
	set_idt(idt);
}



void code_segment_insert(uint64_t * ins, uint64_t * target){
	uint64_t templ = 1<<41|1<<43|1<<44|1<<47;		//rw,executable,nonsys,present
	target* = templ|((ins*)<<48>>32)|((ins*)>>16<<56>>32)|((ins*)>>56<<56)|2;
};
namespace 64bit_sysseg_types{
	#define LDT 0x2
	#define AVLTSS 0x9
	#define BUSYTSS 0xB
	#define TSS_SIZE 0x68
}
void gdt_insert(ustd_t type,uint64_t * ins, uint64_t * target){
	uint64_t sysseg_template = type<<40|1<<47|1<<53|1<<55;	//
	target* = sysseg_template|((ins*)<<48>>32)|((ins*)>>16<<56>>32)|((ins*)>>56<<56)|2;	//piping the pointer as BASE and 2 page length, because
}
void build_tss(uint64_t * tss){
	Kingmem * mm = get_kingmem_object(void);
	ustd_t i;
	for (i = 0; i < 3; ++i){
		tss[i] = mm->get_free_identity(1,pag.SMALLPAGE);
		mm->manipulate_phys(tss[i],1,pag.SMALLPAGE);
	}
	++i;
	for (ustd_t g = i+7 = 0; i < g; ++i){
		tss[i] = mm->get_free_identity(1,pag.SMALLPAGE);
		mm->manipulate_phys(tss[i],1,pag.SMALLPAGE);
	}
	uint16_t * iopb = tss+10;				//io permissions bitmap to make ports unusable by userspace (reminder in rflags ringzero)
	iopb* = iopb-tss+4;
	iopb += 2;
	for (ustd_t j = 0; j < MAX16BIT/sizeof(iopb*); ++j){
		iopb[j] = MAX16BIT;
	}
}
void setup_gdt(void){
	Kingmem * mm = get_kingmem_object(void);
	void * gdt = mm->get_free_identity(MAX16BIT/4096,pag.SMALLPAGE);
	mm->manipulate_phys(gdt,MAX16BIT/4096,pag.SMALLPAGE);

	setup_idt(void);

	for (ustd_t i = 256; i < processors_number; ++i){
		code_segment_insert(NULL,target);		//you can just pass null i think?
		target += 16;
	}

	using namespace 64bit_sysseg_types;
	ustd_t processors_number = get_processors_number(void);
	ustd_t pass = TSS_SIZE*processors_number;
	if (pass%4096){ pass = pass/4096+1;}
	else { pass = pass/4096;}
	void * current_tss = mm->get_free_identity(pass,pag.SMALLPAGE);
	mm->manipulate_phys(current_tss,pass,pag.SMALLPAGE,SET);
	uint64_t * target = gdt+32*16;
	for (ustd_t i = 256; i < processors_number; ++i){
		build_tss(current_tss+4);				//first 4 of TSS are reserved, for some reason
		gdt_insert(AVLTSS,current_tss,target);			//2 pages, see above
		target += 16;
		current_tss += TSS_SIZE;
	}

	ustd_t i;
	for (i = 7; i < MAXPROCESSES; ++i){	//setting up the local descriptor tables, 1 per process
		void * tss = mm->get_free_identity(2,pag.SMALLPAGE);
		mm->manipulate_phys(tss,2,pag.SMALLPAGE,SET);
		gdt_insert(LDT,tss,target);
		target += 16;
	}
	set_gdt(gdt,i*16);
}
