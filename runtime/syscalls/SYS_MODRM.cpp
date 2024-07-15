/*
	Takes an an argument a module directory with files in the following order:
	code	classcodes	models

	Loads it and the info files, attaches the driver to DriversGod and the info file indexes to the driver structure
	the driver will be usable when the next device enumeration gets done
*/

void modinsert(ustd_t dirindex){
	Virtual_fs * vfs; get_vfs_object(vfs);
	DisksKing * dking; get_disksking_object(dking);
	Kingmem * mm; get_kingmem_object(mm);

	Driver * driv = &vfs->descriptions[vfs->descriptions[dirindex]->children[0]];
	Storage * classcodes =  &vfs->descriptions[vfs->descriptions[dirindex]->children[1]];
	Storage * models =  &vfs->descriptions[vfs->descriptions[dirindex]->children[2]];

	ustd_t processor_id = mm->stream_init(void);
	driv->shared_contents = mm->get_free_identity(driv->meta.length,pag.SMALLPAGE);	//NOTE HARDENING
	classcodes->shared_contents = mm->get_free_identity(classcodes->meta.length,pag.SMALLPAGE);
	models->shared_contents = mm->get_free_identity(models->meta.length,pag.SMALLPAGE);
	__non_temporal mm->calendar[processor_id] = 0;

	dking->stream_init(void);
	dking->read(driv->disk,driv->diskpos,driv->shared_contents,driv->meta.length,pag.SMALLPAGE);
	dking->read(classcodes->disk,classcodes->diskpos,classcodes->shared_contents,classcodes->meta.length,pag.SMALLPAGE);
	dking->read(models->disk,models->diskpos,models->shared_contents,models->meta.length,pag.SMALLPAGE);
	__non_temporal dking->calendar[processor_id] = 0;

	driv->classcodes = classcodes;
	driv->models = models;

	DriversGod * drivgod; get_driversgod_object(drivgod);
	drivgod->stream_init(void);
	Driver ** ptr = drivgod->pool_alloc(1);
	__non_temporal drivgod->calendar[processor_id] = 0;
	ptr* = driv;

	(driv->code->start_offset + driv_code)(void);
}
