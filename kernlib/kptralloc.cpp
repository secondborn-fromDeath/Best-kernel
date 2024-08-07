auto ** kptralloc(ustd_t number){
	Kingpointer * kptr = get_kingpointer_object(NUH);
	return kptr->pool_alloc(1);
}
void kptrfree(auto * section, ustd_t number){
	Kingpointer * kptr = get_kingpointer_object(NUH);
	kptr->pool_free(section,number);
}
