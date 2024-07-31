/*
Creating the IO context, devices can be inserted without drivers but drivers cant without devices....*/
void enact_IO_context(void ** driver_pointers, efimap_returns * data){
	load_all_devices(void);
	ustd_t drivnum = 0;
	for (ustd_t i = data->map_size; i; --i){
		if (data->map->vm_pointer == driver_pointer[drivnum]){
			++drivnum;
			Storage * fdriv = vfs->ramcontents_to_description(driver_pointers[drivnum]->image_pointer,driver_pointers[drivnum]->image_size);
			drivgod->pool_alloc(1) = fidriv;
		}
	}
	load_all_devices(void);
}
