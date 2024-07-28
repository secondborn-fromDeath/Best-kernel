ustd_t modinsert(ustd_t drivindex){
	Virtual_fs * vfs = get_vfs_object(void);
	Kingmem * mm = get_kingmem_object(void);
	Kingptr * kptr = get_kingpointer_object(void);

	Directory * dev = vfs->descriptions[1];
	dev->children = kptr->pool_realloc(dev->children_number,dev->children_number+1,dev->children);
	fiDriv * driv = vfs->pool_alloc(1);
	dev->children->children_number = driv;
	++dev->children_number;
	driv->runtime = &kprc->pool[exec(drivindex)];

	DriversGod * relgod = get_driversgod_object(void);
	for (ustd_t i = 0; i < driv->d->code->typerec; ++i){
		if !(relgod->ckarray[driv->d->code->types[i]]){ return NULL;}
		relgod = relgod->pool[driv->d->code->types[i]]
	}
	fiDriv ** ptr = drivgod->pool_alloc(1);
	ptr* = driv;

	driv->runtime->workers[0]->type = thread_types::DRIVER;
	run_ringthree(driv->runtime->children[0]);		//dw about the timer
}
