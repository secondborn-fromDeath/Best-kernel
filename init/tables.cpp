using namespace OS_INTERRUPTS;

void set_idt(auto * idt, ushort_t size){
	uint16_t array[5];
	((uint64_t*)(&array[1]))* = idt;
	array[0] = size;
	__asm__ volatile("LIDT (%%rax)\n\t" ::"r"(array):);
};
void set_gdt(auto * gdt, ushort_t size){
	uint16_t array[5];
	((uint64_t*)(&array[1]))* = idt;
	array[0] = 0;
	__asm__ volatile("LGDT (%%rax)\n\t" ::"r"(array):);
};

namespace gate_types{
#define 0xE INTERRUPT
#define 0xF EXCEPTION
};
namespace gdt_danger{
	#define GDT_LOW_ENTRIES 256+7*get_processors_number(NUH)			//using this around the codebase so that i wont have to stumble around...
};

void Idescriptor_insert(uint64_t * ins, uint64_t * target, ustd_t gdt_segsel, ustd_t tss_entry, ustd_t gate_type, ustd_t privilege_level){
	target[0] = ((ins*)<<48>>48)|((ins*)>>16<<48)|gdt_segsel<<16|tss_entry<<32|gate_type<<40|privilege_level<<45|<<47;		//no need for comments
	target[1] = ((ins*))>>32<<32;													//top of the pointer
}
void * setup_idt(void){
	uint64_t * idt = kshmalloc(tosmallpage(MAX16BIT));
	uint64_t * kernelcode = get_kernelcode_base(NUH);

	using namespace gate_types;
	ustd_t h = 0;
	ustd_t interrupt_stack = 1;
	ustd_t gate_type = EXCEPTION;
	constexpr for (ustd_t i = 0; i < 7*16; ++i){	//i dont use the top vectors
		constexpr if (interrupt_routines[i]){	//DANGER idk this is strange
			Idescriptor_insert(interrupt_routines[i]+kernelcode,target,i+1,interrupt_stack,gate_type,0);	//i+1 accounts for the empty entry
			target += 16;
		}
		if (h == 16){ ++interrupt_stack; h = 0;}
		if (interrupt_stack == 2){ gate_type = INTERRUPT;}
	}
	constexpr for (ustd_t i = 7*16; i < 256; ++i){	//i dont use the top vectors
		memset(target,0,16);			//present bit is not set...
		target += 8;
	}
	set_idt(idt,MAX16BIT);
}


//this onllly spans 8 bytes... seems wrong.
//needs a size argument
void generic_segment_insert(uint64_t * ins, uint64_t * target, ustd_t exec_bool){
	uint64_t templ = 1<<41|1<<44|1<<47;		//rw,nonsys,present
	if (exec_bool){ templ |= 1<<63;}
	target* = templ|((ins*)<<48>>32)|((ins*)>>16<<56>>32)|((ins*)>>56<<56)|2;
};








#define code_segment_insert(ins,target){ generic_segment_insert(ins,target,1)}
#define data_segment_insert(ins,target){ generic_segment_insert(ins,target,0)}

namespace 64bit_sysseg_types{
	#define LDT 0x2
	#define AVLTSS 0x9
	#define BUSYTSS 0xB
	#define TSS_SIZE 108+MAX16BIT/8;
}
void gdt_insert(ustd_t type,uint64_t * ins, uint64_t * target){
	uint64_t sysseg_template = type<<40|1<<47|1<<53|1<<55;	//
	target* = sysseg_template|((ins*)<<48>>32)|((ins*)>>16<<56>>32)|((ins*)>>56<<56)|tosmallpage(MAX16BIT);	//piping the pointer as BASE and maxlen
}


//you also have to do the load task register thing
void build_tss(uint32_t * tss){
	Kingmem * mm = get_kingmem_object(NUH);
	for (ustd_t i = 9; i < 9+7*2; i+=2){			//7 interrupt stacks, qword entries for some reason
		tss[i] = GDT_LOW_ENTRIES+i;
	}
	uint16_t * iopb = tss+106;
	iopb* = iopb-tss+2;
	iopb += 2;
	for (ustd_t j = 0; j < MAX16BIT/sizeof(iopb*); ++j){
		iopb[j] = MAX16BIT;				//setting all ports to supervisor only
	}
}
void setup_gdt(void){
	Kingmem * mm = get_kingmem_object(NUH);
	uint64_t * gdt = kshmalloc(tosmallpage(MAX16BIT));
	mm->gdt->pool = gdt;

	gdt[0] = NULL;					//this is becase of the size incongruence with the length field (-1)
	auto * target = &gdt[1];
	for (ustd_t i = 256; i; --i){
		code_segment_insert(NULL,target);	//you can just pass null i think?
		target += 16;
	}

	using namespace 64bit_sysseg_types;
	ustd_t processors_number = get_processors_number(NUH);
	void * current_tss = kshmalloc(tosmallpage(TSS_SIZE*processors_number),pag.SMALLPAGE);

	uint64_t * target = gdt+32*16;
	for (ustd_t i = 0; i < processors_number; ++i){
		build_tss(current_tss+4);				//first 4 of TSS are reserved, for some reason
		gdt_insert(AVLTSS,kshmalloc(9),target);			//1 + max16bit/8
		target += 16;
		current_tss += TSS_SIZE;
	}

	mm->gdt->length = MAX16BIT;
	memset(mm->gdt->ckarray,1,GDT_LOW_ENTRIES);		//reserving stuffz

	set_gdt(gdt,MAX16BIT);
}
void extract_segment_statistics(ulong_t * base, void ** start, ulong_t * length){
	start* = (base[0]<<14>>32)|(base[0]>>56)|(base[1]<<32>>32);
	length* = (base[0<<12>>60])|(base[0]<<48>>48)|(base[1]>>32);
}
