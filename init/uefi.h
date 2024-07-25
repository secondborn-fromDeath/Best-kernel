/*
These are to be called in 64bit mode
*/


struct efiheader{
	uint64_t signature;
	uint32_t revision;
	uint32_t headersize;
	uint32_t crc32;
	uint32_t pad;
};
struct efimap_returns{
	uint64_t * mapsize;
	void ** map;
	uint32_t * mapkey;
	uint32_t descriptor_size;
	uint32_t descriptor_version;
};
struct efimap_descriptor{
	void * phys_start;
	void * virt_start;
	uint32_t pages_number;
	uint32_t memtype;
	uint32_t descriptor_size;
	uint32_t pad;
};
void * get_bootservices_table(void * efisystab){
	void ** h = efisystab+sizeof(efiheader)+68;
	return h*;
}

volatile void uefi_wrapper(auto * function, auto one,auto two,auto three,auto four, void * additional, ustd_t adds_num){
	/*
	Pushing extra arguments, loading the first 4 argument registers, calling with 16byte alignment, popping things off the stack
	*/
	__asm__ volatile(
	"PUSHq	%%r8\n\t"
	"PUSHq	%%r9\n\t"
	"PUSHq	%%rbp\n\t"							//frame
	"MOVq	%%rsp,%%rbp\n\t"
	"MOVq	40(%%rbp),%%rax\n\t"						//pointer
	"MOVq	48(%%rbp),%%rcx\n\t"						//argsnum
	"uefiwrap_addargs:"
	"TESTq	%%rcx,%%rcx\n\t"
	"JZ	uefiwrap_call\n\t"
	"MOVq	(%%rax,%%rcx,8),%%rdx\n\t"
	"PUSHq	%%rdx\n\t"
	"DECq	%%rcx\n\t"
	"JMP	uefi_addargs\n\t"
	"uefiwrap_call:\n\t"
	"MOVq	(%%rbp),%%rax\n\t"
	"MOVq	8(%%rbp),%%rcx\n\t"						//callconv first 4 arguments...
	"MOVq	16(%%rbp),%%rdx\n\t"
	"MOVq	24(%%rbp),%%r8\n\t"
	"MOVq	32(%%rbp),%%r9\n\t"
	"CALL	%%rax\n\t"
	"MOVq	48(%%rbp),%%rcx\n\t"						//cleaning the stack
	"uefiwrap_popping:\n\t"
	"TESTq	%%rcx,%%rcx\n\t"
	"JZ	uefiwrap_return\n\t"
	"POPq	%%rax\n\t"
	"DECq	%%rcx\n\t"
	"JMP	uefiwrap_popping\n\t"
	"uefiwrap_return:\n\t"
	"POPq	%%rbp\n\t"							//restoring caller state
	"POPq	%%r9\n\t"
	"POPq	%%r8\n\t"
	"RET\n\t"
	);
}


//in 4096 byte pages
void * uefi_allocate_pages(ustd_t pages_number, ulong_t * bootservices_table){
	auto * allocate_pages = (bootservices_table+sizeof(efiheader))[16/8]
	void * ret;
	uefi_wrapper(allocate_pages,0,0,pages_number,&ret,0,0);	//allocate_any, reserve
	return ret;
}
void * uefi_map_pages(ustd_t pages_number, void * request, ulong_t * bootservices_table){
	auto * allocate_pages = (bootservices_table+sizeof(efiheader))[16/8]
	void * ret;
	uefi_wrapper(allocate_pages,2,0,pages_number,&ret);	//allocate_address, reserve			DANGER the reserved type
	return ret;
}
void get_memmap(efimap_returns * returns, ulong_t * bootservices_table){
	auto * uefi_get_memory_map = (bootservices_table+sizeof(efiheader))[32/8];
	returns->mapsize = 512*512*4096;
	returns->map = uefi_allocate_pages(512);	//big enough that it never fails
	uefi_wrapper(uefi_get_memory_map,&returns->mapsize,&returns->map,&returns->mapkey,&returns->descriptor_size,&returns->descriptor_version,1);
	returns->mapsize /= returns->descriptor_size;
}
void exit_boot_services(void * image_handle, uint32_t mapkey, ulong_t * bootservices_table){
	auto * uefi_exbootserv = (bootservices_table+sizeof(efiheader))[208/8];
	uefi_wrapper(uefi_exbootserv,imagehandle,mapkey,0,0,0,0);
}
struct loadfile_returns{
	char * file;
	ulong_t length;
};
ustd_t loadfile(uchar_t * name, ulong_t * bootservices_table, loadfile_returns * returns){
	auto * loadfile = (bootservices_table+sizeof(efiheader))[176/8];
	uint8_t policy = 0;
	uint32_t sourcesize = 512*512*4096;
	void * rethandle;
	ustd_t pages = 1;
	void * pointer = uefi_allocate_pages(1);
	while (uefi_wrapper(loadfile,policy,imagehandle,name,pointer,sourcesize,&rethandle,1) != EFI_SUCCESS){
		++pages;
		pointer = mrealloc(pointer,pages,pag.SMALLPAGE,0);
		if !(pointer){ return (ustd_t)pointer;}
	}
	returns->file = pointer;
	returns->lenght = sourcesize;	//DANGER efi doesnt do a pointer here, kind of weird
}

namespace uefi_memtypes{	//dont ask to make sense of this because it simply makes no sense and i want no part of it.
#define STRONG_UNCACHEABLE 0x1
#define WRITE_COMBINING 0x2
#define WRITETHROUGH 0x4
#define WRITEBACK 0x6
#define WEAK_UNCACHEABLE 0x8
#define WRITEPROTECT 0x1000
#define SUPPORTS_READ_PROTECTION 0x2000
#define SUPPORTS_EXECUTION_PROTECTION 0x4000
#define PERSISTENT_MEMORY 0x8000
#define RELIABLE_MEMORY 0x100000
#define READ_ONLY 0x200000
#define SPECIAL_PURPOSE 0x400000
#define SUPPORTS_CRYPTOGRAPHY 0x800000
#define NEEDS_VIRTUALMAPPING 0x8000000000000000
#define READ_ISA_MASK 0x0FFFFFFFFFFFFFFF			//i simply ignore this because i am only doing x64 right now and the above are more than enough
};
namespace uefi_memtypes_ISA_mask;
