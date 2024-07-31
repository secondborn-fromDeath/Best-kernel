//checking if apic is enabled, if not shutting down because it cant be enabled and i do not support PIC
void ensure_interrupts_mode(void){
	ustd_t bottom;
	ustd_t c = 0x1B;
	ustd_t top;
	rdmsr(c,bottom,top);

	if !(bottom & 1<<11){
		shutdown(void);}
}

void assign_vector_by_irline(ustd_t irline, ustd_t vector){
	IOapicGod * iogod = get_ioapicgod_object(void);
	for (ustd_t p = 0; p < iogod->count; ++p){
		if (iogod->ckarray[p]){
			++true_cnt;
			IOapic * io = &iogod->pool[p];
			ustd_t val = irline + io->global_base;
			if !(val > io->global_base+get_max_redir_entries(io->pointer)){
				assign_vector(io->pointer,irline-io->global_base,vector);
			}
		}
	}
}

//computing the vectors, sorting ioapics by range and piping into the appropriate one's redirection tables
//NOTE STANDARDS, this handles one pin.
void assign_vectors(void){
	Directory * dev = get_vfs_object(void)->descriptions[get_kontrol_object(void)->dev_index];
	IOapicGod * iogod = get_ioapic_object(void);

	ustd_t offset = 64;	//system + mine
	for (ustd_t i = 1; i < dev->children->count+1; ++i){	//have to offset...
		offset += (256-64)/i;
		assign_vector_by_irline(dev->irline,offset);
	}
	init_timer(void);
}
