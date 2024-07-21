ustd_t get_processor_id(void){
	Kontrol * ctrl = get_kontrol_object(void);
	return ((ustd_t *) ctrl->lapic_override)[0x20/4];
}
Processor * get_processor_pointer(void){
	ProcessorsGod * processors = get_processorsgod_object(void);
	return processors[get_processor_id(void)];
}
