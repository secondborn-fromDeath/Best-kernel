//all kings have a mutex

Class ProcessorsGod{ Processor structs[256];};
Class Kontrol{
	void * kernelcode;
	struct mchk mchk_info;
	struct efi_functions efifuncs;
	ustd_t memory_meridian;			//this is a counter for scheduler entries, when it gets to 5 you run the memory evaluation for vfs

	King kings[12];		//this became so fucking bloated jesus
};
enum kings{ MEMORY,PROCESS,THREAD,VIRTUAL_FS,DESCRIPTOR,POINTER,DISK,SHMEMPOOL};





Class King{
	ustd_t MUTEX;
	ustd_t RELEASE;
	ustd_t length;
	uchar_t * ckarray;
	auto * pool;
	uchar_t calendar[256];
};

	auto * pool_alloc(King * k, ustd_t length){
		ustd_t g = 0;
		for (ustd_t i = 0; i < k->length; ++i){
			if (k->ckarray[i] == 0){
				++g;
			}else{ g = 0;}
			if (g == length){
				return k->pool + sizeof(k->pool*)*i;	//type generic
			}
		}
		return NULL;
	}
	void pool_free(King * k, void * section, ustd_t length){
		for (ustd_t g = (section - k->pool)/sizeof(k->pool*); g < length; ++g){ k->ckarray[g] = 0;}
	}
	auto * pool_realloc(King * k, void * section, ustd_t prevlen, ustd_t newlen){
		Kshm * shm; get_shm_object(shm);
		ustd_t processor_id = stream_init(kings.SHMEMPOOL);
		void * backup = shm->pool_alloc(sizeof(k->pool*)*prevlen/4096+1);
		__non_temporal shm->calendar[processor_id] = 0;		//freeing early because safe pipe
		memcpy(backup,section,sizeof(k->pool*)*prevlen);
		k->pool_free(section,prevlen);
		void * ret = pool_alloc(k,newlen);
		memcpy(ret,backup,sizeof(k->pool*)*prevlen);
		shm->free(backup,sizeof(k->pool*)*prevlen/4096+1);
		return ret;
	}

	/*
	NOTE in case you are ever in an architecture without the "send to everyone but me" option you can send the interrupt to the highest processor
	which will then send things cascading down, receivers interrupt, unwind the stack, check if they were executing something within the protected range
	of the mutex and then go back to doing their thing
	*/
	ustd_t stream_init(ustd_t king_index){
		Kontrol * ctrl; get_kontrol_object(ctrl);
		King * king = ctrl->kings[king_index];
		for (ustd_t i = 0; i < 256; ++i){		//reading from the calendar to see if other processors are holding the mutex
			if (king->calendar[i == 255]){
				__asm__(
				"clflush (%%rax)\n\t"
				"clflush 64(%%rax)\n\t"
				"clflush 128(%%rax)\n\t"
				"clflush 196(%%rax)\n\t"
				::"r"(king->calendar):
				);
				i = 0;
			}
		}
		ustd_t interrupt_mask = 1<<14 | 1<<15 | 3<<18;		//assert, level triggered, all excluding self
		ustd_t processor_id = ((ustd_t *)0xFEE0020)* >>56;

		((ustd_t *)0xFEE0300)* = interrupt_mask | king->MUTEX;	//sent
		king->calendar[processor_id] = 255;
		((ustd_t *)0xFEE0300)* = interrupt_mask | king->RELEASE;

		return processor_id;
	}
	void memreq_down_handler(void){__asm__("HLT\n\t");}
	void memreq_up_handler(void){__asm__("add $16,%%rsp\n\t""iret\n\t");}	//ss,cs,ip,sp



Class DisksGod : King{ ustd_t * disks; ustd_t * swapdisks};	//indexes into vfs
Class Kingmem{
	void * vm_ram_table;
	void * phys_ram_table;
	ustd_t paging;
	ulong_t sizeof_ramdisk;
	ulong_t used_memory;	//as the ratio of total memory to this decreases you are going to swap more aggressively, starting from 50% used
};
Class Kingprocess : King{ Process * pool : King.pool;}
Class Kingthread : King{ Thread * pool : King.pool;}
Class Kingptr : King{ auto ** pool : King.pool;}
Class Kshm : King{ struct pag{ uchar_t [4096];} * pool : King.pool;}	//needed for things like realloc
Class Kingdescriptor : King{ Descriptor * pool : King.pool;}
Class Virtual_fs : King{ File * descriptions : King.pool;}
Class DriversGod : King{ Driver ** pool : King.pool;}
Class DriverProcessGod : King{ Runtime_driver * pool : King.pool;}	//this keeps them all
Class SchedKing : King;							//this mutex runs for the entirety of the scheduler
