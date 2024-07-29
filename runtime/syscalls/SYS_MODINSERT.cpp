//doing recursion on the type field of the header for which module the new one wants to help
//DANGER DANGER this is some insane polimorphism
File ** recurse_modules(ustd_t * type, ustd_t typerec){
	DriversGod * drivgod = get_driversgod_object(void);

	File * rel = drivgod->pool[file->d->code->type[0]];
	for (ustd_t i = 1; i < file->d->code->typerec; ++i){
		if (rel->d->length < file->d->code->type[i]){
			drivgod->pool_free(file->d->double_link,1);
			return NULLPTR;
		}
		rel = rel->pool[driv->d->code->types[i]];
	}
}
ustd_t modinsert(ustd_t drivindex){
	Virtual_fs * vfs = get_vfs_object(void);
	Kingmem * mm = get_kingmem_object(void);
	Kingptr * kptr = get_kingpointer_object(void);
	ProcessKing * prcking = get_processking_object(void);

	DriversGod * drivgod = get_driversgod_object(void);

	fiDriv * file = &vfs->descriptions[drivindex];
	file->d->runtime = &kprc->pool[exec(drivindex)];
	if (file->d->runtime = prcking->pool){ return NULLPTR;}		//aka if exec returned NULLPTR
	file->d->double_link = drivgod->pool_alloc(1);			//section... pool_free()...
	file->d->double_link* = &vfs->descriptions[drivindex];		//putting the pointer in drivgod

	if (file->d->runtime->code->identification == driver_kinds::MODULE){
		auto ** rel = recurse_modules(&file->d->runtime->code->type,file->d->runtime->code->typerec);
		if (rel*){	//slot was already occupied, fucking off...
			drivgod->pool_free(file->d->double_link,1);
			return NULLPTR;
		}
		else{
			rel* = file;
		}
	}
	else{	//assumed to be DEVICE_DRIVER
		file->d->double_link = drivgod->pool_alloc(1);
		file->d->double_link* = file;
	}

	file->d->runtime->workers[0]->type = thread_types::DRIVER;
	run_ringthree(file->d->runtime->workers->pool[0]);		//dw about the timer
}
