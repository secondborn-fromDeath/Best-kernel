Thread * get_thread_object(void){
	ustd_t processor_id = get_processor_id(void);
	return &get_threadking_object(void)->pool[get_processorsgod_object(void)->pool[processor_id].current_thread];		//lol
}
Process * get_calling_process(void){
	return get_thread_object(void)->parent;
}
