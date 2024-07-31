//this takes a problem and looks for a specified solution
File * poll_solutions(drivQuirk * caller, ulong_t signature){
	for (ustd_t k = 0; k < caller->length; ++k){
		if (caller->pool[k]->d->runtime->code->signature == signature){
			return caller->pool[k];
		}
	}
	return NULLPTR;
}
//doing recursion on the type field of the header for which module's problem the new one wants to help
//DANGER DANGER this is some insane polimorphism
drivQuirk ** recurse_modules(ustd_t * type, ustd_t typerec){
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
	if !(file->d->double_link){ DRIVKILL(file);}
	file->d->double_link* = &vfs->descriptions[drivindex];		//putting the pointer in drivgod

	if (file->d->runtime->code->identification == driver_kinds::MODULE){
		drivQuirk ** rel = recurse_modules(&file->d->runtime->code->type,file->d->runtime->code->typerec);
		rel->d->pool_alloc(1) = file;
	}

	memcpy(&file->meta.name,&file->d->runtime->code->name,64);

	Thread * drivthread = file->d->runtime->workers->pool[0];
	file->d->pool = vfs->pool_alloc(file->d->runtime->code->problems_number);	//array of quirks...
	if !(file->d->pool){ DRIVKILL(file);}
	for (ustd_t o = 0; o < file->d->runtime->code->problems_number; ++o){
		drivthread->pool[o]->d->hash_type = hash_types::DOUBLE_POINTER;		//solutions are dynamically allocated to problems
	}
	drivthread->count = file->d->runtime->code->problems_number;
	drivthread->type = thread_types::DRIVER;

	run_ringthree(drivthread);		//dw about the timer
}

void SYS_MODINSERT(ustd_t desc){
	Thread * thread = get_thread_object(void);
	Process * process = thread->parent;

	CONDITIONAL_SYSRET(thread,((desc > process->descs->length)||(process->descs->ckarray[desc] == 0)),1);
	module_ctl(process->descs->pool[desc]->findex);
}
