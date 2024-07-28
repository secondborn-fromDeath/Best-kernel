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
	void * gdt;
	void * framebuffer;
	uint64_t framebuffer_size;
};
class ACPI_driver;

enum hashtypes{ POINTER,DOUBLE_POINTER,};
class King : Hash{
	uchar_t calendar;

	ustd_t type;
	ustd_t length;		//total length
	ustd_t count;		//currently occupied slots
	uchar_t * ckarray;
	auto * pool;

	/*
	NOTE in case you are ever in an architecture without the "send to everyone but me" option you can send the interrupt to the highest processor
	which will then send things cascading down, receivers interrupt, unwind the stack, check if they were executing something within the protected range
	of the mutex and then go back to doing their thing
	*/
	ustd_t stream_init(void){
		volatile __non_temporal while (this->calendar);

		brothers_sleep(void);
		__non_temporal this->calendar = 255;
		brothers_wake(void);
	}

	auto * pool_alloc(ustd_t reqlen){
		this->stream_init(void);
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
		__non_temporal this->calendar = 0;
		return NULLPTR;
	}
	void pool_free(void * section, ustd_t length){
		for (ustd_t g = (section - this->pool)/sizeof(this->pool*); g < length; ++g){ this->ckarray[g] = 0;}
		this->count -= length;
	}
	auto * pool_realloc(void * section, ustd_t prevlen, ustd_t newlen){
		Kshm * shm = get_shm_object(void);
		shm->stream_init(void);
		void * backup = shm->pool_alloc(sizeof(this->pool*)*prevlen/4096+1);
		__non_temporal shm->calendar = 0;					//safe pipe
		if !(backup){ return NULLPTR;}
		memcpy(backup,section,sizeof(this->pool*)*prevlen);
		this->pool_free(section,prevlen);
		void * ret = pool_alloc(k,newlen);
		memcpy(ret,backup,sizeof(this->pool*)*prevlen);
		shm->free(backup,sizeof(this->pool*)*prevlen/4096+1);
		return ret;
	}
};

#define (x)(y)segment_alloc(size){	\
	(xypool_alloc(size)-xypool)/sizeof(xypool*)	\
}
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
	void * vm_ram_table;
	void * phys_ram_table;
	ustd_t paging;
	GDT gdt;
	ulong_t sizeof_ramdisk;
	ulong_t used_memory;	//as the ratio of total memory to this decreases you are going to swap more aggressively, starting from 50% used
};
class Kingprocess : King{ Process * pool : King.pool;}
class Kingthread : King{ Thread * pool : King.pool;}
class Kingdescriptor : King{ Descriptor * pool : King.pool;}
class Virtual_fs : King{ File * descriptions : King.pool;}
class Kingptr : King{ auto ** pool : King.pool;}
class Kshm : King{ struct pag{ uchar_t [4096];} * pool : King.pool;}	//needed for things like realloc		also base for Swap
class DriversGod : King{ fiDriv ** pool : King.pool;}
class DriverProcessGod : King{ Runtime_driver * pool : King.pool;}	//this keeps them all
class Taskpimp : Kingthread;
