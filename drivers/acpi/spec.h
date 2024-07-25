/*
In this file I am declaring structures based on the acpi specification, for ease of use

table 5.5 ~has the table identifiers~, it doesnt even exist
*/

struct ACPI_RSDP{	//headerless
	char signature[8];
	uint8_t checksum;		//first 20 bytes, see acpi 1.0 checksum
	char OEMid[6];
	uint8_t revision;
	uint32_t RSDT_ptr;		//32bit pointer
	uint32_t length;
	void * XSDT_ptr;
	uint8_t ext_checksum;		//entire table
	uint8_t pad;
};

struct ACPI_SYSHEADER{
	char signature[4];
	uint32_t table_length;
	uint8_t revision;
	uint8_t checksum;		//entire table, header included
	char OEMid[6];
	char OEMtable_id[8];		//function names for aml stuff
	uint32_t OEM_revision;		//idk
	uint32_t creator_ID;		//the acpi compiler i guess
	uint32_t creator_revision;
};

struct ACPI_RSDT{
	ACPI_SYSHEADER head;
	uint32_t entries[];		//32bit pointers		head.table_length determined the number of entries
};
struct ACPI_XSDT{
	ACPI_SYSHEADER head;
	void * entries[];		//64bit pointers		head.table_length determined the number of entries
};



enum ACPI_MADT_flags{ NOPCAT_COMPAT,YESPCAT_COMPAT};
struct ACPI_MADT{
	ACPI_SYSHEADER head;
	uint32_t lapic_pointer;		//for all processors
	uint32_t flags;			//look up
	uint8_t structures[];		//sizes are awkward...
};
//page 128 btw
enum ACPI_MADT_STRUCTURES{
	LOCALAPIC,IOAPIC,IRSOURCE_OVERRIDE,NMI_SOURCE,LAPIC_NMI,LAPIC_PTROVERRIDE,IO_STREAMAPIC,PLATFORM_IRSOURCES,LOCAL_X2APIC,LX2APIC_NMI,
	GICC,GICD,GICITS,	//arm shit
	MULTIPROCESSOR_WKUP,COREPIC,LIO_PIC,EIO_PIC,MSI_PIC,BRIDGEIO_PIC,LPC_PIC,MAX_VALID,	//anything above MAX_VALID is reserved
};
struct MADT_LAPIC{
	uint8_t type;
	uint8_t length;
	uint8_t acpi_processor_uid;	//processor object nonsense
	uint8_t apic_id;		//the one you pipe at the top of +0x310
	uint32_t flags;			//0 is wether the processor is active, 1 is wether the processor in online capable		these dont act as NMI switches btw...
	void structures[];		//not pointers, structures
};
struct MADT_IOAPIC{
	uint8_t type;
	uint8_t length;
	uint8_t ioapic_id;	//dont get confused with irlines order...
	uint8_t pad;
	uint32_t ioapic_ptroverride;	//you know the pcat skip a page from FEC...
	uint32_t global_system_interrupt_override;
};

struct ACPI_FADT{
	ACPI_SYSHEADER head;
	uint32_t firmware_ctrl;		//pointer to FACS
	uint32_t DSDT_pointer;
	uint8_t pad;
	uint8_t preferred_PM_profile;
	uint16_t SCI_IRLINE;
	uint32_t SCI_CMD_PORT;
	uint8_t ACPI_ENABLE;
	uint8_t ACPI_DISABLE;
	uint8_t S4BIOS_REQ;
	//many other important fields i am not doing right now...
};





namespace ACPI_SIGNATURES{
	#define MADT			0x41504943		//standing for APIC
	#define RSDP_PTR_		0x5253442050545220
	#define FADT			0x46414350		//stands for FACP
};

