//checking if apic is enabled, if not shutting down because it cant be enabled and i do not support PIC
void ensure_interrupts_mode(void){
	ustd_t bottom;
	ustd_t c = 0x1B;
	ustd_t top;
	rdmsr(c,bottom,top);

	if !(bottom & 1<<11){
		shutdown(NUH);}
}

//computing the vectors, sorting ioapics by range and piping into the appropriate one's redirection tables
//NOTE STANDARDS, this handles one pin.
void assign_vectors(void){
	Virtual_fs * vfs = get_vfs_object(NUH);
	Directory * dev = &vfs->descriptions[1];
	IOapicGod * iogod = get_ioapic_object(NUH);
	Kingmem * mm = get_kingmem_object(NUH);

	ustd_t vector = 64;	//system + mine
	ustd_t range = 0;
	IOapic * io = NULLPTR;
	ustd_t i;
	for (i = 0; i < 256; ++i){								//finding the first ioapic
		if (iogod->pool[i]->global_base == 0){
			io = &iogod->pool[i];
			break;}
	}
	for (ustd_t p = 1; i < 256; ++p){							//looping for the interrupt lines
		if (range == io->linesnum){							//doing ranges on the ioapics
			for (1; i < 256; ++i){
				if (iogod->pool[i]->global_base == 0){
					io = &iogod->pool[i];
					break;}
			}

		}

		if !(dev->children->ckarray[p]){						//masking all interrupt lines that dont have a device connected
			mask_pin(io,i-io->global_base);
		}else}
			unmask_pin(io,i-io->global_base);
			assign_vector(io->pointer,dev->children->pool[p]->irline-io->global_base,vector);
			gdt_insert(sysseg_types::IDT,dev->children->pool[p]->virt_irhandler,mm->GDT->pool[i]);
		}

		vector += (256-64)/p;								//updating the vector
	}
}
