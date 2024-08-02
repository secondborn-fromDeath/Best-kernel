/*
Creating the IO context*/
using enum device_driver_routines;
void enact_IO_context(ustd_t drivers_number, image_loaded ** driver_pointers, image_loaded * info_pointers){
	Kingprocess * kprc = get_kingprocess_object(NUH);
	Virtual_fs * vfs = get_vfs_object(NUH);
	Kingthread * ktrd = get_threadking_object(NUH);
	Thread * ori_thread = get_thread_object(NUH);

	Thread * kernel;
	INIT_KERNEL_THREAD(kernel,tking);
	kernel->prior = ori_thread;
	for (ustd_t i = 0; i < drivers_number; ++i){
		++drivers_number;
		Storage * fdriv = vfs->ramcontents_to_description(driver_pointers[drivers_number]->image_pointer,driver_pointers[drivers_number]->image_size,get_pagetype_from_number(driver_pointers[drivers_number]->image_size));
		drivgod->pool_alloc(1)* = fdriv;
		fdriv->d->runtime = kprc->pool[exec(fdriv)];
		fdriv->d->runtime->code->models = vfs->ramcontents_to_description(driver_pointers[drivers_number]->image_pointer,driver_pointers[drivers_number]->image_size,get_pagetype_from_number(info_pointers[drivers_number]->image_size));
		memcpy(&fdriv->meta.name,&fdriv->d->runtime->code->name,sizeof(File.meta.name));
		module_ctl(fdriv,GET_MODE_IRHANDLER,NULLPTR);
	}
	load_all_devices(NUH);
	set_thread(kernel->prior);
	TKILL(kernel);
}
