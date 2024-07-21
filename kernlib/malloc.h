//this is to be used by in-kernel functions

void * malloc(ustd_t pages_number, ustd_t pagetype){
	Kingmem * mm = get_kingmem_object(void);
	void * ret = mm->get_free_identity(pages_number,pagetype);
	mm->manipulate_phys(ret,pages_number,pagetype,SET);
	return ret;
}
