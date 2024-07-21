/*
Reminder to check the bootlaoder code, anyway in kontrol i put them in order for ease of reading
the base control is cr6
*/

auto * get_base(void){	//wants to be a function beacuse of the serialization
	void * base;
	__asm__(
	"MOV	%%cr6,%%rax\n\t"
	::"r"(base):);
	return base;
}

namespace singletons{					//anon namespace has scope limited to this file
	#define ACPI_OFFSET	0
	#define KONTROL_OFFSET		ACPI_OFFSET+sizeof(ACPI_driver)
	#define KINGMEM_OFFSET		KONTROL_OFFSET+sizeof(Kontrol)
	#define PROCESSOR_OFFSET	KINGMEM_OFFSET+sizeof(Kingmem)
	#define IOAPIC_OFFSET		PROCESSOR_OFFSET+sizeof(ProcessorsGod)
	#define PROCESS_OFFSET		IOAPIC_OFFSET+sizeof(IOapicGod)
	#define THREAD_OFFSET		PROCESS_OFFSET+sizeof(Kingprocess)
	#define DESCRIPTOR_OFFSET	THREAD_OFFSET+sizeof(Kingthread)
	#define VFS_OFFSET		DESCRIPTOR_OFFSET+sizeof(Kingdescriptor)
	#define POINTER_OFFSET		VFS_OFFSET+sizeof(Virtual_fs)
	#define SHM_OFFSET		POINTER_OFFSET+sizeof(Kingptr)
	#define DRIVERS_OFFSET		SHM_OFFSET+sizeof(Kingshm)
	#define RUNTIME_DRIVERS_OFFSET	DRIVERS_OFFSET+sizeof(DriversGod)
	#define FULLSINGLETONS		RUNTIME_DRIVERS_OFFSET+sizeof(DriverProcessGod)
};
ì
ACPI_Driver * get_acpi_object(void){
	return get_base(void)+singletons::ACPI_OFFSET;
}
Kontrol * get_kontrol_object(void){
	return get_base+singletons::KONTROL_OFFSET;
}
Kingmem * get_kingmem_object(void){
	return get_base+singletons::KINGMEM_OFFSET;
}
ProcessorsGod * get_processorsgod_object(void){
	return get_base(void)+singletons::PROCESSOR_OFFSET;
}
IOapicGod * get_ioapic_object(void){
	return get_base(void)+singletons::IOAPIC_OFFSET;
}
Kingprocess * get_kingprocess_object(void){
	return get_base(void)+singletons::PROCESS_OFFSET;
}
Kingthread * get_kingthread_object(void){
	return get_base(void)+singletons::THREAD_OFFSET;
}
Kingdescriptor * get_kingdescriptor_object(void){
	return get_base(void)+singletons::DESCRIPTOR_OFFSET;
}
Virtual_fs * get_vfs_object(void){
	return get_base(void)+singletons::VFS_OFFSET;
}
Kingptr * get_kingpointer_object(void){
	return get_base(void)+singletons::POINTER_OFFSET;
}
Kshm * get_kshm_object(void){
	return get_base(void)+singletons::SHM_OFFSET;
}
DriversGod * get_driversgod_object(void){
	return get_base(void)+singletons::DRIVERS_OFFSET;
}
DriverProcessGod * get_runtimedrivers_object(void){
	return get_base(void)+singletons::RUNTIME_DRIVERS_OFFSET;
}
