/*
Functions:
* means a function is NOT called directly by the kernel

		.env_init(RSDP * RSDP)
	*	.get_GPE_info()				fills GPE registers info and control methods pointers
		.build_processors_and_ioapic_array()	fills structures with information from MADT
		.Fill_DEV()				creates the "virtual" (ie. not pci-addressable) Device structures and fills all Device structures with pointers to their aml object
		.Execute_Method()			...
		.Execute_VirtualMethod()		above but goes through/*
Functions:
* means a function is NOT called directly by the kernel

		.env_init(RSDP * RSDP)
	*	.get_GPE_info()				fills GPE registers info and control methods pointers
		.build_processors_and_ioapic_array()	fills structures with information from MADT
		.Fill_DEV()				creates the "virtual" (ie. not pci-addressable) Device structures and fills all Device structures with pointers to their aml object
		.Execute_Method()			...
		.Execute_VirtualMethod()		above but goes through southbridge
		.Shutdown()				...
		.gpeHandler()				reads the gpes that were signed in the GPEn_BLK registers and executes the aml methods
		.get_global_lock()			NOT_ASYNC gets ownership of _GLK

if you ever forget anything CORE look up words that buzz from pag.258 and the FACS and FADT structures
reminder about the x2apic structure for where the processor structures and thermal zones stuff are
anyway if you forget the aml syntax, reminder there is no recursion.
*/

enum trigger_modes{ LEVEL,EDGE,};
Class ACPI_driver{
	ACPI_RSDP * RSDP;
	struct{
	uint16_t gpe1_baseport;
		uint8_t gpe1_length;
		uint8_t gpe2_length;
		uint16_t gpe2_baseport;
		gpe_method * methods;
	} GPE_info;



	void env_init(void);

	void * find_toplevel_table(ustd_t mask){
		ustd_t it_inc;
		ACPI_SYSHEADER * table = sizeof(ACPI_SYSHEADER);
		if (this.RSDP->revision > 0){ it_inc = 2; table += this.RSDP->XSDT_ptr;}
		else{ it_inc = 1; table += this.RSDP->RSDT_ptr;}

		for (ustd_t i = 0; i < (table->length-sizeof(ACPI_SYSHEADER))/8;; ++i){
			ACPI_SYSHEADER * test = ((ACPI_SYSHEADER**)table)[0];
			if (table->head->signature == mask){
				return test;
			}
		}
		return NULLPTR;
	}
	/*
	A myriad of get_ XXX methods...*/
	void build_processors_and_ioapic_array(ProcessorGod * prgod, IOapicGod * iogod){
		//getting to the madt table in the root table entries
		ACPI_MADT * madt = get_toplevel_table(ACPI_SIGNATURES::APIC);

		Kontrol * ctrl; get_kontrol_object(ctrl);
		ctrl->lapic_pointer = madt->lapic_override;

		//finding all of the processor and io apic structures and making the entries into the arrays
		for (auto * table = &madt->structures; table < madt->head->length; table += table->head->length){
			if (table->head->type == ACPI_MADT_STRUCTURES.LOCALAPIC){
				prgod->pool[table->apic_id]->online_capable |= table->flags>>1;	//everything from 2-31 is reserved 0?
				++prgod->count;

			}
			else if (table->head->type == ACPI_MADT_STRUCTURES.IOAPIC){
				iogod->pool[table->ioapic_id]->global_base = table->global_system_interrupt_base;
				iogod->pool[table->ioapic_id]->pointer = table->ioapic_ptroverride;
				iogod->pool[table->ioapic_id]->linesnum = get_max_redir_entries(table->ioapic_ptroverride);
				++iogod->count;
			}
		}
	}
	//getting the acpi control interrupt global line number and assigning a vector to it (SCI)
	//attaching an entry to iret() for now lolmao
	void attach_SCI(void * fadt, ustd_t vector){
		ACPI_FADT * fadt = get_toplevel_table(ACPI_SIGNATURES::FADT);

		ACPI_FACS * facs = fadt->firmware_ctrl;
		assign_vector_by_irline(facs->SCI_IRLINE,OS_INTERRUPTS::ACPI_CONTROL);
	}
	/*
	Seeing wether the ps2 controller is present*/
	ustd_t test_ps2_controller(void){
		char * fadt = get_toplevel_table(ACPI_SIGNATURES::FADT);
		return (fadt[109] & 2);
	}
	/*
	Shutting down the machine
		retrieves the obejcts in the acpi device namespace
		outw(PM1a_CNT + (SPL_TYPa|SLP_EN))
		and boom machine is powered off, honestly idk why this wouldnt just be a normal standardized port but whatever, midwits let companies do whatever they want

*/
	void shutdown(void);
};
 southbridge
		.Shutdown()				...
		.gpeHandler()				reads the gpes that were signed in the GPEn_BLK registers and executes the aml methods
		.get_global_lock()			NOT_ASYNC gets ownership of _GLK

if you ever forget anything CORE look up words that buzz from page 258 and the FACS and FADT structures
reminder about the x2apic structure for where the processor structures and thermal zones stuff are
anyway if you forget the aml syntax, reminder there is no recursion at all and idiots want to brainwash you with the scope opcode
*/

enum trigger_modes{ LEVEL,EDGE,};
Class ACPI_driver{
	ACPI_RSDP * RSDP;
	struct{
	uint16_t gpe1_baseport;
		uint8_t gpe1_length;
		uint8_t gpe2_length;
		uint16_t gpe2_baseport;
		gpe_method * methods;
	} GPE_info;



	void env_init(void);

	void * find_toplevel_table(ustd_t mask){
		ustd_t it_inc;
		ACPI_SYSHEADER * table = sizeof(ACPI_SYSHEADER);
		if (this.RSDP->revision > 0){ it_inc = 2; table += this.RSDP->XSDT_ptr;}
		else{ it_inc = 1; table += this.RSDP->RSDT_ptr;}

		for (ustd_t i = 0; i < (table->length-sizeof(ACPI_SYSHEADER))/8;; ++i){
			ACPI_SYSHEADER * test = ((ACPI_SYSHEADER**)table)[0];
			if (table->head->signature == mask){
				return test;
			}
		}
		return NULLPTR;
	}
	/*
	A myriad of get_ XXX methods...*/
	void build_processors_and_ioapic_array(ProcessorGod * prgod, IOapicGod * iogod){
		//getting to the madt table in the root table entries
		ACPI_MADT * madt = get_toplevel_table(ACPI_SIGNATURES::APIC);

		Kontrol * ctrl; get_kontrol_object(ctrl);
		ctrl->lapic_pointer = madt->lapic_override;

		//finding all of the processor and io apic structures and making the entries into the arrays
		for (auto * table = &madt->structures; table < madt->head->length; table += table->head->length){
			if (table->head->type == ACPI_MADT_STRUCTURES.LOCALAPIC){
				prgod->pool[table->apic_id]->online_capable |= table->flags>>1;	//everything from 2-31 is reserved 0?
				++prgod->count;

			}
			else if (table->head->type == ACPI_MADT_STRUCTURES.IOAPIC){
				iogod->pool[table->ioapic_id]->global_base = table->global_system_interrupt_base;
				iogod->pool[table->ioapic_id]->pointer = table->ioapic_ptroverride;
				iogod->pool[table->ioapic_id]->linesnum = get_max_redir_entries(table->ioapic_ptroverride);
				++iogod->count;
			}
		}
	}
	//getting the acpi control interrupt global line number and assigning a vector to it (SCI)
	//attaching an entry to iret() for now lolmao
	void attach_SCI(void * fadt, ustd_t vector){
		ACPI_FADT * fadt = get_toplevel_table(ACPI_SIGNATURES::FADT);

		ACPI_FACS * facs = fadt->firmware_ctrl;
		assign_vector_by_irline(facs->SCI_IRLINE,OS_INTERRUPTS::ACPI_CONTROL);
	}
	/*
	Seeing wether the ps2 controller is present*/
	ustd_t test_ps2_controller(void){
		char * fadt = get_toplevel_table(ACPI_SIGNATURES::FADT);
		return (fadt[109] & 2);
	}
	/*
	Shutting down the machine
		retrieves the obejcts in the acpi device namespace
		outw(PM1a_CNT + (SPL_TYPa|SLP_EN))
		and boom machine is powered off, honestly idk why this wouldnt just be a normal standardized port but whatever, midwits let companies do whatever they want

*/
	void shutdown(void);
};
