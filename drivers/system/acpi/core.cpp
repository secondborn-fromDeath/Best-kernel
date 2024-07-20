/*
Key is to be interpreted with the parent environment
We are donig simple recursion with multiple tables
*/


Class ACPI_driver{
	ACPI_RSDP * RSDP;



	void env_init(void){
		 Kingmem * mm; get_kingmem_object(mm);
		mm->stream_init(void);
		this.map = mm->get_free_identity(1,MIDPAGE); //getting 2 mb
		mm->manipulate_phys(this.map,1,SET); __non_temporal mm->calendar = 0;
	}

	/*
	A myriad of get_ XXX methods...*/
	void build_processors_and_ioapic_array(Processor * processors_array, IOapic * ioapics_array){
		ustd_t it_inc;
		ACPI_SYSHEADER * table = sizeof(ACPI_SYSHEADER);
		if (this.RSDP->revision > 0){ it_inc = 2; table += this.RSDP->XSDT_ptr;}
		else{ it_inc = 1; table += this.RSDP->RSDT_ptr;}

		//getting to the madt table in the root table entries
		ACPI_MADT * madt;
		while (table->head->signature != ACPI_SIGNATURES.APIC){
			table += table->head->length;
		}
		madt = table;

		Kontrol * ctrl; get_kontrol_object(ctrl);
		ctrl->lapic_pointer = madt->lapic_override;

		//finding all of the processor and io apic structures and making the entries into the arrays
		for (table = &madt->structures; table < madt->head->length; table += table->head->length){
			if (table->head->type == ACPI_MADT_STRUCTURES.LOCALAPIC){
				processors_array[table->apic_id]->online_capable |= table->flags>>1;	//everything from 2-31 is reserved 0?
			}
			else if (table->head->type == ACPI_MADT_STRUCTURES.IOAPIC){
				ioapics_array[table->ioapic_id]->global_interrupt_base = table->global_system_interrupt_base;
			}
		}
	}

	/*
	Shutting down the machine
		retrieves the obejcts in the acpi device namespace
		outw(PM1a_CNT + (SPL_TYPa|SLP_EN))
		and boom machine is powered off, honestly idk why this wouldnt just be a normal standardized port but whatever, midwits let companies do whatever they want

*/
	void shutdown(void);

	/*
	Southbridge stuff*/
};
