//checking if it is not enabled, if shutting down because it cant be enabled
void ensure_interrupts_mode(void){
	ustd_t bottom;
	ustd_t c = 0x1B;
	ustd_t top;
	rdmsr(c,bottom,top);

	if !(bottom & 1<<11){
		shutdown(void);
	}
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

	ustd_t up_down = 0;	//wether the previous operation rounded up or down
	ustd_t offset = 32;
	for (ustd_t i = 0; i < dev->numberof_children; ++i){
		offset += (256-32)/dev->numberof_children;
		if !(up_down){ offset += (256-32)%dev->numberof_children; ++up_down}
		else{ up_down = 0;}
		vectors[i] = offset;
		assign_vector_by_irline(dev->irline,offset);
	}
}
