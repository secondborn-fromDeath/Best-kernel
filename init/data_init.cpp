/*
	In this file i am laying down all data structures for the runtime kernel, it functions as extended documentation too

	Conceptualizing memory usage in the kernel:
		Kernel keys:
				ringzero server pagetree
				physical ram table
				singletons like Kontrol and Kingmem
		Each of the following have their own pools (because they are just arrays of structures):
				process structures
				thread structures
				virtual filesystem structures
				per-processor processor structure
		Userspace memory pool (the vast majority of memory):
				processes' code, stacks, other segments i offer
				random process data
		Kernel memory pool


The order of things:
	>parse memory map and make phys_ram_table containing lowmem and ramsticks		NOTE highmem is only accessible through PRIVATE KERNEL functions
	>keys, kernelspace shm, object arrays, userspace shm


OBTAINABILITY PROOF FOR EACH OF THE VARIABLES BEING PASSED:
	maplen		=	from bootloader with firmware calls
	memmap		=	above
	paging		=	cpuid, leaf 7, sub=0, bi t 16 of ecx.

For now i am going to waste some pages of memory by using only the ramsticks (basically skipping
over past the reset vector so i am sure i am never doing anything on the pcie configurations spaces)
*/
//NOTE DANGER this is specific to x64


//DANGER COMPILE-TIME
__attribute__((optimization("O0"))) struct uefimap_descriptor{
	ustd_t type;
	void * phys_start;
	void * vm_start;	//dont be confused, this is uefi-app only
	ulong_t pages_number;
	ulong_t attribute;
};

	void data_init(ustd_t * maplen, struct uefimap_descriptor * memmap, ustd_t paging){
		//skipping to above the pcie configs and the reset vector
		ustd_t i;
		for (i = 0; memmap[i].phys_start < 0xffffffff; ++i);
		ulong_t sizeof_memory = memmap[i].pages_number;
		void * startof_memory = memmap[i].phys_start;

		//because of uefi constraints i have to do this thing where i am making a pagetree that can fit in 4096 bytes and then
		//exitbootservices(), then make another tree, move there and then make the final tree above the reset vector
		void * next_tree = sizeof_memory*8/512/512+3*8 + startof_memory;
		if (paging == 5){ next_tree += 8;}
		ulong_t * tree = memmap[i].vm_start;
		ustd_t g;
		for (g = 0; g != (paging-3)*2; g += 2){
			tree[g] = 1|2|8|4096|512;
			tree[g+1] = 0;			//should be good enough
		}
		tree[g] = 1|2|8|4096|512 | (next_tree)<<14;
		tree[g+1] = 1|2|8|4096|512 | (next_tree+ONEGIG)<<14;	//guaranteed good enough?
		tree[g+2] = 0;
		tree[g+3] = 1;	//2mb
		tree[g+4] = 0;
		tree[g+5] = 1;	//4kb
		tree[g+4] = 0;
		//setting the temporary pagetree root
		__asm__ volatile(
		"mov %%cr3,%%rcx\n\t"
		"or %%rax,%%rcx\n\t"
		"xor %%rax,%%rcx\n\t"
		"or %%rax,%%rcx\n\t"
		"mov %%rcx,%%cr3\n\t"
		"jmp +0\n\t"
		::"r"(next_tree):"%rcx"
		);

		tree = NULL;		//holy shit dereferencing a null pointer
		for (g = 0; g != (paging-3)*2; g += 2){
			tree[g] = 1|2|8|4096|512;
			tree[g+1] = 0;
		}
		tree[g] = 1|2|8|4096|512 | (final_tree)<<14;		//same as above
		tree[g+1] = 1|2|8|4096|512 | (final_tree+ONEGIG)<<14;
		tree[g+2] = 0;
		tree[g+3] = 1;	//2mb
		tree[g+4] = 0;
		tree[g+5] = 1;	//4kb
		tree[g+4] = 0;
		//setting the second temporary pagetree root
		__asm__ volatile(
		"mov %%cr3,%%rcx\n\t"
		"or %%rax,%%rcx\n\t"
		"xor %%rax,%%rcx\n\t"
		"or %%rax,%%rcx\n\t"
		"mov %%rcx,%%cr3\n\t"
		"jmp +0\n\t"
		::"r"(next_tree):"%rcx"
		);

		tree = NULL;
		#define final_tree startof_memory
		for (g = 0; g != (paging-3)*2; g += 2){
			tree[g] = 1|2|8|4096|512;
			tree[g+1] = 0;
		}
		for (ulong_t raise = 0; raise < sizeof_memory+MAX32BIT; raise += ONEGIG){	//mapping IO too.
			tree[g] = 1|2|8|4096|512 | (raise)<<14;		//mapping memory in gigabytes because its shorter
			++g;
			tree[g] = 0;
			++g;
		}
		tree[g+2] = 0;
		tree[g+3] = 1;	//2mb
		tree[g+4] = 0;
		tree[g+5] = 1;	//4kb
		tree[g+4] = 0;
		//setting the second temporary pagetree root
		__asm__ volatile(
		"mov %%cr3,%%rcx\n\t"
		"shl $51,%%rcx\n\t"
		"shr $51,%%rcx\n\t"
		"shl $13,%%rax\n\t"
		"or %%rax,%%rcx\n\t"
		"mov %%rcx,%%cr3\n\"
		"jmp +0\n\t"
		::"r"(final_tree):"%rcx"
		);

		//tree is NULL already
		#define get_kontrol_object(x)		x = 8196;
		#define get_kingmem_object(x)		x = 8196+sizeof(Kontrol);
		#define get_kingthread_object(x)	x = 8196+sizeof(Kontrol);+sizeof(Kingmem);
		#define get_kingprocess_object(x)	x = 8196+sizeof(Kontrol);+sizeof(Kingmem)+sizeof(Kingthread);
		#define get_vfs_object(x)		x = 8196+sizeof(Kontrol);+sizeof(Kingmem)+sizeof(Kingthread)+sizeof(Kingprocess);
		#define get_processor_object(x,y)	x = 8196+sizeof(Kontrol);+sizeof(Kingmem)+sizeof(Kingthread)+sizeof(Kingprocess)+sizeof(Virtual_fs)+sizeof(Processor)*y;

		void * sp = 8196+sizeof(Kontrol)+sizeof(Kingmem)+sizeof(Processor)*256+sizeof(Process)*MAX_PROCESSES+sizeof(Thread)*MAX_THREADS*MAX_PROCESSES+sizeof(Virtual_fs);
		__asm__ volatile("mov %%eax,%%esp\n\t"	::"r"(sp):"%rsp");

		Kontrol ctrl;
		Kingmem mm;
		Kingthread something_one;
		ctrl.threads = &something_one;
		Kingprocess something_two;
		ctrl.processes = something_two;
		Virtual_fs something_three;
		ctrl.vfs = &vfs;
		Processor processors[256];	//enough in 100% of cases
		memset(processors,0,constexpr ctrl-processors);

		//stuff to be passed on the kernel command line
		uchar_t mut_one[256]; something_one.calendar = mut_one;
		uchar_t ki_one[MAX_THREADS*MAX_PROCESSES]; something_one.ckarray = ki_one;
		Thread whore_one[MAX_THREADS*MAX_PROCESSES]; something_one.pool = whore_one;
		uchar_t mut_two[256]; something_two.calendar = mut_two;
		uchar_t ki_two[MAX_PROCESSES]; something_two.ckarray = ki_two;
		Process whore_two[MAX_PROCESSES]; something_two.pool = whore_two;
		uchar_t mut_three[256]; something_three.calendar = mut_three;
		uchar_t ki_three[MAX_EXT4]; something_three.ckarray = ki_three;
		File whore_three[MAX_EXT4]; something_tree.pool = whore_three;
		memset(whore_three,0,constexpr processors-whore_three);

		uchar_t physram[sizeof_memory/8];
		memset(physram,1,(sp/4096+1)/8);
		memset(physram+sp/8,0,sizeof_memory/8-(sp/4096+1)/8);

		mm.dir_entries[0] = sizeof_memory/512/512;
		mm.dir_entries[1] = sizeof_memory/512;
		mm.dir_entries[2] = sizeof_memory;

		void * erm = sizeof_ram*4096;
		__asm__ volatile(
		"mov %%rax,%%rsp\n\t"
		"ret\n\t"
		::"r"(erm):"%rsp"
		);	//i need a stack for the sipi sequence and i need the prior one to not be popped for obvious reasons
	}
