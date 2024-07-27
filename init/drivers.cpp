/*
Here you need to load the config file with UEFi and then bind them to driversgod and the virtual filesystem

NOTE HARDENING for the filelength
*/

ustd_t get_drivers_number(char * config){
	ustd_t ret = 0;
	for (ustd_t k = 0; config[k] != '\n'; ++k){				//DANGER if eval is before increase...
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
void load_drivers(void * bootservices_table, void ** driver_vmpointers, char * drivconf){
	char string[64];
	string[0] = 0x2F64726976657273;	//"/drivers"
	ustd_t driver_cnt = 0;
	for (ustd_t k = 0; drivconf[k] != '\n'; ++k){				//DANGER if eval is before increase...
		char * drivername = string+8;
		ustd_t j;
		for (j = 0; ((drivername[j] != '\n')&&(drivername[j] != '#'); ++j){
			drivername[j] = drivconf[k];
			++k;
		}
		drivername[j+1] = 0;
		loadfile_args temp;
		driver_vmpointer[driver_cnt] = loadfile(string,bootservices_table,&temp);
		++driver_cnt;

		if (drivconf[k] != '\n'){ while (drivconf[k] != '\n'){ ++k};}
	}
}
//outsourcing uefimain logic because aesthetics
ustd_t initialize_drivers(void * bootservices_table, void ** driver_vmpointers){
	uint64_t drivconf = 0x2F647269762E6366;	// "/driv.cf"
	loadimage_args temp;
	uefi_loadimage(&drivconf,&temp);		//actually check it lol
	ustd_t ret = get_drivers_number(temp->pointer);
	load_drivers(bootservices_table,driver_vmpointers,temp->pointer);
}
