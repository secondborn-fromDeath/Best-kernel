/*
This cleans ALL RESOURCES a process used and deallocates it from the processking array*/
void PKILL(Process * process){
	Kingmem * mm = get_kingmem_object(NUH);
	mm->recursive_free(process,pagetype,process->pagetree);

	Virtual_fs * vfs = get_vfs_object(NUH);
	for (ustd_t i = 0; i < process->descs->length; ++i){
		if (process->descs->ckarray[i] == 0){ continue;}
		char * maps = process->descs->pool[i]->maps;
		if (maps){
			File * file = vfs->descriptions[process->descs->pool[i]->findex];
			for (ustd_t y = 0; y < file->mem->length; ++y){
				if (maps[y]){
					--file->mem->listeners[i];}
			}
			kptrfree(process->descs->pool[i]->maps,512/8);}
		--vfs->descriptions[process->descs->pool[i]->findex]->openers;
	}
	Kingdescriptor * kdesc = get_kingdescriptor_object(NUH);
	kdesc->pool_free(process->descs->pool,process->descs->length);

	void * start;
	ulong_t length;

	extract_segment_statistics(&process->local_descriptor_table->pool[4],&start,&length);			//freeing the heap segment
	free(vmto_phys(process->pagetree,start),tosmallpage(length),pag::SMALLPAGE);

	extract_segment_statistics(&mm->GDT->pool[process->local_descriptor_table->segsel],&start,&length);	//process' ldt from gdt
	free(vmto_phys(process->pagetree,start),tosmallpage(length),pag::SMALLPAGE);

	Kingprocess * kprc = get_kingprocess_object(NUH);
	kprc->pool_free(process);
}
/*
Above but for threads*/
void TKILL(Thread * thread){
	ustd_t segsel = get_segment_selector(thread->state.gs);							//freeing thread local storage
	void * start;
	ulong_t length;
	extract_segment_statistics(&thread_parent->local_descriptor_table->pool[segsel],&start,&length);
	free(vmto_phys(process->pagetree,start),tosmallpage(length),pag::SMALLPAGE);

	thread->parent->children->pool_free(thread->double_link,1);

	Kingthread * ktrd = get_kingthread_object(NUH);
	ktrd->pool_free(thread.1);
}
/*
Drivers*/
void DRIVKILL(fiDriv * driver){
	TKILL(driver->d->runtime->children->pool[0]);
	PKILL(driver->d->runtime);

	Driversgod * drivgod = get_driversgod_object(NUH);
	drivgod->pool_free(driver->d->double_link,1);

	driver->parent->children->pool_free(device->double_link,1);
	Virtual_fs * vfs = get_vfs_object(NUH);
	vfs->pool_free(driver,1);
}
/*
Quirks*/
void QUIRKKILL(drivQuirk * quirk){
	Quirksgod * qgod = get_quirksgod_object(NUH);
	qgod->pool_free(quirk->double_link);
	Virtual_fs * vfs = get_vfs_object(NUH);
	vfs->pool_free(quirk,1);
}
/*
Devices*/
void DEVICEKILL(Device * device){
	TKILL(device->thread);

	Virtual_fs * vfs = get_vfs_object(NUH);
	Directory * dev = &vfs->descriptions[1];
	dev->children->pool_free(device->double_link);
	vfs->pool_free(device,1);
}
