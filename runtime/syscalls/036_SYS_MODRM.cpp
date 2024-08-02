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
	QUIRKKILL(unborn);
}
void modrm_recu(DriversGod * drivgod, Virtual_fs * vfs, QuirksGod * qgod, fiDriv * unborn){
	for (ustd_t i = 0; i < unborn->d->length;;){
		if (unborn->d->ckarray[i]){
			quirkrm(drivgod,vfs,qgod,unborn->d->pool[i]);
			++i;
		}
	}
}
void modrm(File * driv){	//index into the virtual filesystem
	Virtual_fs * vfs = get_vfs_object(void);
	DriversGod * drivgod = get_drivgod_object(void);
	QuirksGod * qgod = get_quirksgod_object(void);

	brothers_sleep(void);
	modrm_recu(drivgod,vfs,qgod,driv);
	DRIVKILL(driv);
	for (ustd_t i = 0; i < qgod->length; ++i){
		if !(qgod->ckarray[i]){ continue;}
		drivQuirk * quirk = &qgod->pool[i];
		for (ustd_t u = 0; quirk->ckarray[i]; ++u){
			if (quirk->pool[u] == driv){ quirk->pool_free(&quirk->pool[u],1);}
	}
	brothers_wake(void);
}

void SYS_MODRM(ustd_t desc){
	Thread * thread = get_thread_object(NUH);
	Process * process = thread->parent;

	CONDITIONAL_SYSRET(thread,process->owner_id != owner_ids::ROOT,1);
	CONDITIONAL_SYSRET(thread,((desc > process->descs->length)||(process->descs->ckarray[desc] == 0)),2);

	Virtual_fs * vfs = get_vfs_object(NUH);
	File * driv = &vfs->descriptions[process->descs->pool[desc]->findex];

	Module_removal_stack * modrm_sp = get_modrm_stack_object(NUH);
	modrm_sp->pool_alloc(driv,1);
	thread->sys_retval = 0;
	SYSRET;
}
