/*
This removes a driver from DriversGod, unloads the files linked to the driver and the driver code, then innescates the device enumeration

*/

void modrm(ustd_t drivindex){
	Virtual_fs * vfs; get_vfs_object(vfs);
	DriversGod * drivgod; get_driversgod_object(drivgod);
	DisksKing * dking; get_disksking_object(dking);

	Driver * ourguy = &vfs->descriptions[drivindex];
	for (ustd_t g = 0; g < drivgod->length; ++g){
		if (ourguy == drivgod->pool[g]){
			drivgod->pool_free(&drivgod->pool[g],1);

			Storage * classcodes = &vfs->descriptions[ourguy->classcodes];
			Storage * models = &vfs->descriptions[ourguy->models];

			ustd_t processor_id = dking->stream_init(void);
			dking->write(classcodes->disk,classcodes->diskpos,classcodes->shared_contents,classcodes->meta.length,pag.SMALLPAGE);
			dking->write(models->disk,models->diskpos,models->shared_contents,models->meta.length,pag.SMALLPAGE);
			dking->calendar[processor_id] = 0;

			vfs->pool_free(ourguy,1);
			vfs->pool_free(classcodes,1);
			vfs->pool_free(models,1);

			broadcast_ipi_allbut_self(REQ_DOWN_HANDLER);	//see kontrol.h and interrupts
			reenact_kernelspace_setup(void);
			broadcast_ipi_allbut_self(REQ_UP_HANDLER);
		}
	}
}
