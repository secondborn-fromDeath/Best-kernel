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
	efimap_descriptor ** map;
	uint32_t * mapkey;
	uint32_t descriptor_size;
	uint32_t descriptor_version;
};
struct efimap_descriptor{
	uint32_t type;
	void * phys_start;
	void * virt_start;
	uint32_t pages_number;
	uint32_t attribute;
	uint32_t descriptor_size;
	uint32_t pad;
};
#define UEFI_LOADED_IMAGE_PROTOCOL 0x5B1B31A1956211d283F00A0C969723B
volatile struct image_loaded{	//page 248, this is peak confusion tbh, osdev are larpers LMAO
	uint32_t revision;
	void * efisystab;
	void * handle;
	void * block_device;
	void * reserved;
	uint32_t loadoptions_size;
	void * loadoptions;
	void * image_base;
	uint64_t image_size;
	uint32_t imagecode_type;	//these are "ENUMS" which is fucking retarded, efi writers are so stupid it is embarrassing...
	uint32_t imagedata_type;
	auto * unload_function;
};
#define EFI_GRAPHICS_OUTPUT_PROTOCOL 9042a9de23dc4a3896fb7aded080516a
struct graphics_protocol{
	auto * query_mode;
	auto * set_mode;
	auto * bit;
	graphics_info * info;
};
struct graphics_info{
	uint32_t maxnode;
	uint32_t mode;
	void * something_i_dont_care_about;
	uint32_t sizeof_above;
	void * physical_framebuffer;
	uint32_t framebuffer_size;
};


void * get_bootservices_table(void * efisystab){
	void ** h = efisystab+sizeof(efiheader)+68;
	return h*;
}
void * get_runtimeservices_table(void * efisystab){
	void ** h = efisystab+sizeof(efiheader)+60;
	return h*;
}

/*
This is a function that allows the C calling convention to interface with the UEFI callign convention
Any argument is passed as a qword*/
volatile void uefi_wrapper(auto * function, uint64_t * args, ustd_t num){
__asm__ volatile(
"PUSHq	%%r8\n\t"				//pushing the incongruent calling convention...
"PUSHq	%%r9\n\t"
"PUSHq	%%r10\n\t"
"PUSHq	%%r11\n\t"
"PUSHq	%%rsi\n\t"				//string instructions
"PUSHq	%%rdi\n\t"
"PUSHq	%%rbx\n\t"				//cleaning the stack
"MOVq	%%rdx,%%rbx\n\t"
"MOVq	%%rcx,%%rsi\n\t"			//I HATE CLOBBERS! LET ME SPECIFY THE REGISTER MANUALLY INSTEAD OF DOING THIS ENUM NONSENSE
"MOVq	%%rdx,%%r10\n\t"
"TESTq	%%r10,%%r10\n\t"
"JZ	uefiwrapper_call\n\t"
"MOVq	64(%%rsi),%%rcx\n\t"
"TESTq	%%r10,%%r10\n\t"
"JZ	uefiwrapper_call\n\t"
"MOVq	72(%%rsi),%%rdx\n\t"
"TESTq	%%r10,%%r10\n\t"
"JZ	uefiwrapper_call\n\t"
"MOVq	80(%%rsi),%%r8\n\t"
"TESTq	%%r10,%%r10\n\t"
"JZ	uefiwrapper_call\n\t"
"MOVq	88(%%rsi),%%r9\n\t"
"MOVq	%%rsp,%%rdi\n\t"
"uefiwrapper_additionals_loop:"
"TESTq	%%r10,%%r10\n\t"
"JZ	uefiwrapper_call\n\t"
"SUBq	$8,%%rsp\n\t"
"MOVSq	\n\t"					//assuming direction flag is clear...
"JMP	uefiwrapper_additionals_loop\n\t"
"uefiwrapper_call CALL %%rax\n\t"
"MOVq	%%rbx,%%rcx\n\t"			//see "cleaning the stack"
"STD	\n\t"					//settign the direction flag and copying back the return values because
"REPnz	MOVsq"					//si and di are protected by uefi callconv
"LEAq	(%%rsp,%%rbx,8),%%rsp\n\t"
"POPq	%%rbx\n\t"
"POPq	%%rdi\n\t"
"POPq	%%rsi\n\t"
"POPq	%%r11\n\t"
"POPq	%%r10\n\t"
"POPq	%%r9\n\t"
"POPq	%%r8\n\t"
::"r"(function),"r"(args),"r"(num):);
}


//in 4096 byte pages
void * uefi_allocate_pages(ustd_t pages_number, ulong_t * bootservices_table){
	auto * allocate_pages = (bootservices_table+sizeof(efiheader))[16/8]
	uint64_t array[] = [NULLPTR,NULLPTR,pages_number,NULLPTR,];			//reserve, allocate any
	uefi_wrapper(allocate_pages,array,4);
	return array[3];
}
void * uefi_map_pages(ustd_t pages_number, void * request, ulong_t * bootservices_table){
	auto * allocate_pages = (bootservices_table+sizeof(efiheader))[16/8]
	uint64_t array[] = [2,NULLPTR,pages_number,request];			//reserve, allocate any
	uefi_wrapper(allocate_pages,array,4);
	return array[3];
}
void get_memmap(efimap_returns * returns, ulong_t * bootservices_table){
	auto * uefi_get_memory_map = (bootservices_table+sizeof(efiheader))[32/8];
	returns->mapsize = 512*512*4096;
	returns->map = uefi_allocate_pages(512);				//big enough that it never fails
	uint64_t array[] = [returns->mapsize,returns->map,NULLPTR, NULLPTR,];	//would corrupt the calling stack otherwise because the last 2 things are 32bit
	uefi_wrapper(uefi_get_memory_map,array,4);
	returns->descriptor_size = array[2];
	returns->descriptor_version = array[3];
	returns->mapsize /= returns->descriptor_size;
}
void exit_boot_services(void * image_handle, uint64_t mapkey, ulong_t * bootservices_table){
	auto * uefi_exbootserv = (bootservices_table+sizeof(efiheader))[208/8];
	uefi_wrapper(uefi_exbootserv,&image_handle);					//DANGER "optimization"
}
image_loaded * loadfile(char * name, ulong_t * bootservices_table, void * parent_handle){
	auto * loadfile = (bootservices_table+sizeof(efiheader))[176/8];
	void * newimage_handle;
	uint64_t array[] = [NULLPTR,parent_handle,name,NULLPTR,NULLPTR,NULLPTR,&newimage_handle,];

	uefi_wrapper(loadfile,array,7);

	char guid[16] = UEFI_LOADED_IMAGE_PROTOCOL;
	(uefi_locate_protocol(array,guid,1))(newimage_handle);
	return ((void **)guid)*;
}
void uefi_locate_protocol(char * guid, ulong_t * bootservices_table){				//destroys you guid
	uefi_wrapper(bootservices_table[280/8],guid,1);
}
void boot_get_framebuffer(auto * boottab, void ** fb_pointer, uint64_t * fb_size){
	char pass[16] = UEFI_GRAPHICS_OUTPUT_PROTOCOL;
	uefi_locate_protocol(pass,boottab);
	fb_pointer* = pass->framebuffer_base;
	fb_size* = pass->framebuffer_size;
}

namespace uefi_memory{
	namespace attributes{
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
	#define FIRMWARE_RUNTIME 0x8000000000000000
	#define READ_ISA_MASK 0x0FFFFFFFFFFFFFFF
	};
	enum boot_types{
		RESERVED,
		LOADER_CODE,
		LOADER_DATA,
		BOOTSERV_CODE,
		BOOTSERV_DATA,
		RUNSERV_CODE,
		RUNSERV_DATA,
		__CONVENTIONAL__,	//the one actually usable.
		UNUSABLE,
		ACPI_RECLAIM,
		ACPI_FIRMWARE_RESERVED,
		MMIO,
		MAPPED_SERIAL_PORTS,
		MICROCODE,
		PERSISTENT_MEMORY,	//usable NONVRAM
	};
	enum runtime_types;
};
namespace uefi_memtypes_ISA_mask;
