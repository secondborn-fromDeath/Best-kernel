class ProcessorsGod{ Processor a[256];};
class IOapic{
	ustd_t global_interrupt_base;	//there is no guarantee of geometry by the ioapic id, read acpi spec MADT
};
class IOapicGod{ IOapic a[16];};
class Kontrol{
	void * kernelcode;
	struct mchk mchk_info;			//acpi shit
	struct efi_functions efifuncs;
	ustd_t memory_meridian;			//this is a counter for scheduler entries, when it gets to 5 you run the memory evaluation for vfs
	void * lapic_pointer;
	ushort_t shutdown_port;			//port number that triggers shutdown SMI
	ustd_t device_directory_index;		//into vfs
	File * framebuffer;
	ulong_t devenum_millis;
};
class ACPI_driver;



enum hashtypes{ POINTER,DOUBLE_POINTER,};
class Hash{
	ustd_t type;
	ustd_t length;		//total length
	ustd_t count;		//currently occupied slots
	uchar_t * ckarray;
	auto * pool;

	auto * alloc(ustd_t intense, ustd_t reqlen){
		ustd_t g = 0;
		ustd_t i = 0;
		auto * ret;
		for (1; i < this->length; ++i){
			if (this->ckarray[i] == 0){
				++g;
			}else{ g = 0;}
			if (g == reqlen){
				this->count += g;
				return this->pool + sizeof(this->pool)*i;			//probably needs to get turned into an argument to get the compiler to emit an argument
			}
		}
		if (this->type == hashtypes.DOUBLE_POINTER){		//kindly asking kingptr for more pointers
			ret = get_kingptr_object(void)->pool_realloc(this->pool,this->length,this->length+reqlen,sizeof(this->pool*));
			if (tryn){
				this->pool = tryn;}
				tryn += this->length*sizeof(this->pool*);
				this->length += reqlen;
			}
		}
		return NULLPTR;
	}
	void free(void * section, ustd_t length){
		for (ustd_t g = (section - this->pool)/sizeof(this->pool*); g < length; ++g){ this->ckarray[g] = 0;}
		this->count -= length;
		memset(section,0,length*sizeof(this->pool*));
	}
	auto * realloc(ustd_t intense, void * section, ustd_t prevlen, ustd_t newlen){
		ustd_t lel = sizeof(this->pool*)*prevlen/4096+1;
		void * backup = malloc(lel,pag::SMALLPAGE);
		if !(backup){ return backup;}
		memcpy(backup,section,sizeof(this->pool*)*prevlen);
		this->pool_free(section,prevlen);
		void * ret = pool_alloc(k,newlen);
		memcpy(ret,backup,sizeof(this->pool*)*prevlen);
		free(backup,lel);
		return ret;
	}
};


void mutex(char * calendar){
	__asm__ volatile(
	"MOVq	%%rax,%%rcx\n\t"
	"MOVq	$255,%%rdx\n\t"
	"mutex_loop:\n\t"
	"XORl	%%eax,%%eax\n\t"	//comparing 0 to the calendar
	"LOCK	CMPXCHG (%%rcx),%%dl\n\t"
	"TEST	%%edx,%%edx\n\t"	//testing wether the exchange happened
	"JZ	mutex_broken\n\t"
	"lelul: MOVq $500,%%rax\n\t"	//wasting some time before queueing memory again
	"DEC	%%eax\n\t"
	"TEST	%%eax,%%eax\n\t"
	"JNZ	lelul\n\t"
	"jmp	mutex_loop\n\t"
	"mutex_broken:\n\t"
	::"r"(&this->calendar):);
}
class King : Hash{
	uchar_t calendar;

	/*
	NOTE in case you are ever in an architecture without the "send to everyone but me" option you can send the interrupt to the highest processor
	which will then send things cascading down, receivers interrupt, unwind the stack, check if they were executing something within the protected range
	of the mutex and then go back to doing their thing
	*/
	void stream_init(void){
		mutex(&this->calendar);
	}
	auto * pool_alloc(ustd_t reqlen){
		this->stream_init(void);
		auto * ret = Hash::pool_alloc(reqlen);
		__non_temporal this->calendar = 0;
		return ret;
	}
	auto * pool_realloc(void * section, ustd_t prevlen, ustd_t newlen){
		this->stream_init(void);
		auto * ret = Hash::pool_realloc(section,prevlen,newlen);
		__non_temporal this->calendar = 0;
		return ret;
	}
};



class LDT : King{ struct{ uint64_t[2];} * pool : King.pool;
	ustd_t gdt_index;		//otherwise you need sorting ON THE TRANSLATIONS
};
class GDT : King{struct{ uint64_t[2];} * pool : King.pool;
	auto * translate_ldt_entry(ustd_t index){
		if (index < (get_processors_number(void)+256+1)){ return NULLPTR;}
		ulong_t * ptr = &this->pool[index];
		return (ptr[0]<<32>>48)|(ptr[0]<<25>>58)|(ptr[0]>>56)|ptr[1];	//top is reserved as 0...
	}
};
class Kingmem{
	void * phys_ram_table;
	GDT gdt;
	ulong_t sizeof_memory;
	ulong_t used_memory;	//as the ratio of total memory to this decreases you are going to swap more aggressively, starting from 50% used
};
class Kingprocess : King{ Process * pool : King.pool;}
class Kingthread : King{
	Thread * pool : King.pool;
	auto * pool_alloc(ustd_t reqlen){
		this->stream_init(void);
		Thread * ret = Hash::pool_alloc(reqlen);
		ret->taken = 255;
		__non_temporal this->calendar = 0;
		return ret;
	}
};
class Virtual_fs : King{ File * descriptions : King.pool;}
class Kingptr : King{ auto ** pool : King.pool;}
class DriversGod : King{ fiDriv ** pool : King.pool;}
class Quirksgod : King{ drivQuirk ** pool : King.pool;}
class SyscallsGod{ auto ** functions;}					//these both get dereferenced by templaters
class InterruptsGod{ auto ** functions;}
class Module_removal_stack : King{
	File * pool : King.pool;
};
class Disksking;
class Pci;
