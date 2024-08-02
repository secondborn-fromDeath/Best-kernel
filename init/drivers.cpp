/*
Here you need to load the config file with UEFi and then bind them to driversgod and the virtual filesystem

NOTE HARDENING for the filelength
*/

ustd_t get_drivers_number(char * config){
	ustd_t ret = 0;
	for (ustd_t k = 0; config[k] != '\n'; ++k){
		char * drivername = string+8;
		ustd_t j;
		for (j = 0; ((drivername[j] != '\n')&&(drivername[j] != '#'); ++j){
			drivername[j] = config[k];
		}
		++ret;
		if (config[k] != '\n'){ while (config[k] != '\n'){ ++k};}
	}
	return ret;
}
void load_drivers(void * bootservices_table, image_loaded ** driver_pointers, image_loaded ** info_pointers, image_loaded * drivconf_image){
	char * drivconf = (char *)drivconf_image->image_base;
	char string[64];
	string[0] = 0x2F64726976657273;	//"/drivers"
	ustd_t driver_cnt = 0;
	for (ustd_t k = 0; drivconf[k] != '\n'; ++k){
		char * drivername = string+8;
		ustd_t j;
		for (j = 0; ((drivername[j] != '\n')&&(drivername[j] != '#'); ++j){
			drivername[j] = drivconf[k];
			++k;
		}
		++j;
		drivername[j] = 0;
		loadfile_args temp;
		driver_pointers[driver_cnt] = uefi_loadfile(string,bootservices_table,&temp);
		++driver_cnt;

		//loading the info file
		ulong_t * info = &drivername[j];
		info* = 0x2E696E666F;								//DANGER unaligned access on pointer arithmetic...
		info_pointers[driver_cnt] = uefi_loadfile(string,bootservices_table,&temp);

		if (drivconf[k] != '\n'){ while (drivconf[k] != '\n'){ ++k};}
		if (k -> drviconf_image->image_size){ shutdown(void);}				//bad configuration file...
	}
}
//outsourcing uefimain logic because aesthetics
ustd_t fetch_drivers_basics(void * bootservices_table, image_loaded ** drivconf_image){
	uint64_t drivconf = 0x2F647269762E6366;	// "/driv.cf"
	loadimage_args temp;
	drivconf_image* = uefi_loadimage(&drivconf,&temp);
	return get_drivers_number(temp->pointer);
}
