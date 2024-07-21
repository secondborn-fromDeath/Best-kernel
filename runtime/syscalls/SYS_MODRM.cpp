/*
Recursively removes modules
Requires brothers to be asleep
*/

void quirkrm(DriversGod * drivgod, Virtual_fs * vfs, QuirksGod * qgod, drivQuirk * unborn){
	for (ustd_t i = 0; i < unborn->length;;){
		if (unborn->ckarray[i]){
			modrm_recu(drivgod,vfs,qgod,unborn->pool[i]);
			++i;
		}
	}
	qgod->pool_free(unborn,1);
}
void modrm(DriversGod * drivgod, Virtual_fs * vfs, QuirksGod * qgod, fiDriv * unborn){
	for (ustd_t i = 0; i < unborn->d->length;;){
		if (unborn->d->ckarray[i]){
			quirkrm(drivgod,vfs,qgod,unborn->d->pool[i]);
			++i;
		}
	}
	vfs->pool_free(unborn,1);
	drivgod->pool_free(unborn->double_link,1);
}
void modrm_wrap(ustd_t drivindex){	//index into the virtual filesystem
	Virtual_fs * vfs = get_vfs_object(void);
	DriversGod * drivgod = get_drivgod_object(void);
	QuirksGod * qgod = get_quirksgod_object(void);

	brothers_sleep(void);
	modrm(drivgod,vfs,qgod,&vfs->descriptions[drivindex]);
	brothers_wake(void);
}
