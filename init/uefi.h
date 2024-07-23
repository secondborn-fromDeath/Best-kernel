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
void * get_bootservices_table(void * efisystab){		//called in C conv
	void ** h = efisystab+sizeof(efiheader)+68;
	return h*;
}


void EFIAPI get_memmap(efimap_returns * returns, ustd_t * bootservices_table){
	auto * uefi_get_memory_map = (bootservices_table+sizeof(efiheader))[32/4];
	returns->mapsize = 512*512*4096;
	returns->map = get_free_identity(1,MIDPAGE);		//big enough that it never fails
	(uefi_get_memory_map)(&returns->mapsize,&returns->map,&returns->mapkey,&returns->descriptor_size,&returns->descriptor_version);
	returns->mapsize /= returns->descriptor_size;
}
void EFIAPI exit_boot_services(void * image_handle, uint32_t mapkey, ustd_t * bootservices_table){
	auto * uefi_exbootserv = (bootservices_table+sizeof(efiheader))[208/4];
	(uefi_exbootserv)(imagehandle,mapkey);
}
struct loadfile_returns{
	char * file;
	ulong_t length;
};
char * EFIAPI loadfile(uchar_t * name, ustd_t * bootservices_table, loadfile_returns * returns){
	auto * loadfile = (bootservices_table+sizeof(efiheader))[176/4];
	uint8_t policy = 0;
	uint32_t sourcesize = 512*512*4096;
	void * rethandle;
	ustd_t pages = 1;
	void * pointer = malloc(1,pag.SMALLPAGE);
	//NOTE HARDENING
	while ((loadfile)(policy,imagehandle,name,pointer,sourcesize,&rethandle) != EFI_SUCCESS){
		++pages;
		pointer = mrealloc(pointer,pages,pag.SMALLPAGE);
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
