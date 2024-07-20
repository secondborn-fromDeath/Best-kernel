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
	//that is all you need
};
struct efimap_descriptor{
	void * phys_start;
	void * virt_start;
	uint32_t pages_number;
	uint32_t memtype;
	uint32_t descriptor_size;
	uint32_t pad;
};
namespace uefi_memrel{
	#define MOST_RELIABLE 0x100000
	#define READONLY 0x200000
	#define SPEC_PURP 0x400000
	#define CRYPTO 0x800000
	#define FIRM_RUNTIME 0x8000000000000000
	#define ISA_VALID 0x4000000000000000
	#define ISA_MASK 0x0FFFF00000000000
}
void * get_bootservices_table(void ** efisystab){
	__asm__ volatile(
	"MOVl	(%%esp,%%eax),%%eax\n\t"
	"MOVl	68(%%eax),%%eax\n\t"
	"ret\n\t"
	::"r"(sizeof(efiheader)):);
}
void get_memmap(efimap_returns * returns){
	uint64_t * bootservices_table = get_bootservices_table(void);
	auto * uefi_get_memory_map = (bootservices_table+sizeof(efiheader))[32/4];
	void * memory_map = get_free_identity(1,MIDPAGE);	//so it is big enough that it never fails
	__asm__ (
	"MOVq	%%rdx,%%r15\n\t"	//moving pointer into r15
	"CALL	%%eax\n\t"
	"MOVq	%%rcx,(%%r15)\n\t"
	"MOVq	%%rdx,8(%%r15)\n\t"
	"MOVq	%%r8,16(%%r15)\n\t"
	::"r"(uefi_get_memory_map),"r"(memory_map),"r"(returns):);
}
void exit_boot_services(void * image_handle, uint32_t mapkey){
	uint64_t * bootservices_table = get_bootservices_table(void);
	auto * uefi_exbootserv = (bootservices_table+sizeof(efiheader))[208/4];
	__asm__ (
	"CALL	%%rax\n\t"
	::"r"(uefi_exbootserv),"r"(imagehandle),"r"(mapkey):);
}
