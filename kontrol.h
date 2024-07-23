Class ProcessorsGod{ Processor a[256];};
Class IOapic{
	ustd_t global_interrupt_base;	//there is no guarantee of geometry by the ioapic id, read acpi spec MADT
};
Class IOapicGod{ IOapic a[16];};
Class Kontrol{
	void * kernelcode;
	struct mchk mchk_info;			//acpi shit
	struct efi_functions efifuncs;
	ustd_t memory_meridian;			//this is a counter for scheduler entries, when it gets to 5 you run the memory evaluation for vfs
	void * lapic_pointer;
	ushort_t shutdown_port;			//port number that triggers shutdown SMI
	ustd_t device_directory_index;		//into vfs
};
Class ACPI_driver;

Class Hash{
	ustd_t length;		//total length
	ustd_t count;		//currently occupied slots
	uchar_t * ckarray;
	auto * pool;

	auto * pool_alloc(ustd_t length, ustd_t peasant_length){
		ustd_t g = 0;
		for (ustd_t i = 0; i < this.length; ++i){
			if (this.ckarray[i] == 0){
				++g;
			}else{ g = 0;}
			if (g == length){
				this.count += g;
				return this.pool + sizeof(peasant_length)*i;	//type generic
			}
		}
		return NULL;
	}
	void pool_free(void * section, ustd_t length, ustd_t peasant_length){
		for (ustd_t g = (section - this.pool)/sizeof(peasant_length); g < length; ++g){ this.ckarray[g] = 0;}
		this.count -= g;
	}
	auto * pool_realloc(void * section, ustd_t prevlen, ustd_t newlen, ustd_t peasant_length){
		Kshm * shm; get_shm_object(shm);
		ustd_t processor_id = shm->stream_init(void);
		void * backup = shm->pool_alloc(sizeof(peasant_length)*prevlen/4096+1);
		__non_temporal shm->calendar[processor_id] = 0;		//freeing early because safe pipe
		memcpy(backup,section,sizeof(peasant_length)*prevlen);
		this.pool_free(section,prevlen);
		void * ret = pool_alloc(k,newlen);
		memcpy(ret,backup,sizeof(peasant_length)*prevlen);
		shm->free(backup,sizeof(peasant_length)*prevlen/4096+1);
		return ret;
	}
};
Class King : Hash{
	uchar_t calendar;

	/*
	NOTE in case you are ever in an architecture without the "send to everyone but me" option you can send the interrupt to the highest processor
	which will then send things cascading down, receivers interrupt, unwind the stack, check if they were executing something within the protected range
	of the mutex and then go back to doing their thing
	*/
	ustd_t stream_init(void){
		__non_temporal while (this.calendar != 1);

		brothers_sleep(void);
		__non_temporal this.calendar = 255;
		brothers_wake(void);
	}
};

#define (x)(y)segment_alloc(size){	\
	(xypool_alloc(size)-xypool)/sizeof(xypool*)	\
}
Class LDT : King{ struct{ uint64_t[2];} * pool : King.pool;
	ustd_t gdt_index;		//otherwise you need sorting ON THE TRANSLATIONS
};
Class GDT : King{struct{ uint64_t[2];} * pool : King.pool;
	auto * translate_ldt_entry(ustd_t index){
		if (index < (get_processors_number(void)+256+1)){ return NULL;}
		ulong_t * ptr = &this.pool[index];
		return (ptr[0]<<32>>48)|(ptr[0]<<25>>58)|(ptr[0]>>56)|ptr[1];	//top is reserved as 0...
	}
};
Class Kingmem{
	void * vm_ram_table;
	void * phys_ram_table;
	ustd_t paging;
	GDT gdt;
	ulong_t sizeof_ramdisk;
	ulong_t used_memory;	//as the ratio of total memory to this decreases you are going to swap more aggressively, starting from 50% used
};
Class Kingprocess : King{ Process * pool : King.pool;}
Class Kingthread : King{ Thread * pool : King.pool;}
Class Kingdescriptor : King{ Descriptor * pool : King.pool;}
Class Virtual_fs : King{ File * descriptions : King.pool;}
Class Kingptr : King{ auto ** pool : King.pool;}
Class Kshm : King{ struct pag{ uchar_t [4096];} * pool : King.pool;}	//needed for things like realloc		also base for Swap
Class DriversGod : King{ fiDriv ** pool : King.pool;}
Class DriverProcessGod : King{ Runtime_driver * pool : King.pool;}	//this keeps them all
