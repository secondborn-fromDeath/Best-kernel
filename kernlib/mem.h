//this is to be used by in-kernel functions

void * malloc(ustd_t pages_number, ustd_t pagetype){
	Kingmem * mm = get_kingmem_object(void);
	void * ret = mm->get_free_identity(pages_number,pagetype);
	mm->manipulate_phys(ret,pages_number,pagetype,SET);
	return ret;
}
void mfree(void * pointer, ustd_t pages_number, ustd_t pagetype){
	Kingmem * mm = get_kingmem_object(void);
	mm->manipulate_phys(ret,pages_number,pagetype,CLEAR);
}
//this is slow but whatever
void * mrealloc(void * pointer, ustd_t previous_pages, ustd_t new_pages, ustd_t pagetype, ustd_t backup){
	Kingmem * mm = get_kingmem_object(void);
	void * newp = malloc(new_pages,pagetype);
	if (backup){ memcpy(newp,pointer,get_multi_from_pagetype(pagetype)*previous_pages);}
	mfree(pointer,previous_pages,pagetype);
	return newp;
}

/*
These are macros with a special meaning to be expanded by the compiler
*/
#define memcpy(dest,source,bytes){	\
	for (ustd_t i = 0; i < bytes; ++i){	\
		((uchar_t *)dest)[i] = ((uchar_t *)source)[i];	\
	}	\
}
#define memset(dest,val,bytes){	\
	for (ustd_t i = 0; i < bytes; ++i){	\
		((uchar_t *)dest)[i] = val;	\
	}	\
}
