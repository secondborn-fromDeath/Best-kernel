/*
Creating the IO context*/
void enact_IO_context(ustd_t drivers_number, image_loaded ** driver_pointers, image_loaded * info_pointers){
	Kingprocess * kprc = get_kingprocess_object(void);
	Virtual_fs * vfs = get_vfs_object(void);

	for (ustd_t i = 0; i < drivers_number; ++i){
		++drivers_number;
		Storage * fdriv = vfs->ramcontents_to_description(driver_pointers[drivers_number]->image_pointer,driver_pointers[drivers_number]->image_size,get_pagetype_from_number(driver_pointers[drivers_number]->image_size));
		drivgod->pool_alloc(1)* = fdriv;
		fdriv->runtime = kprc->pool[exec(fdriv)];
		fdriv->runtime->code->models = vfs->ramcontents_to_description(driver_pointers[drivers_number]->image_pointer,driver_pointers[drivers_number]->image_size,get_pagetype_from_number(info_pointers[drivers_number]->image_size));
		memcpy(&fdriv->meta.name,&fdriv->runtime->code->name,sizeof(File.meta.name));
	}
	load_all_devices(void);
}
