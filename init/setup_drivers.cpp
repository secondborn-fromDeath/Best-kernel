/*
Here you need to load the config file with UEFi and then bind them to driversgod and the virtual filesystem
*/

void setup_drivers(void * bootservices_table){
	ulong_t string[8];
	string[0] = 0x2F647269762E6366;		//"/driv.cf"
	string[1] = NULL;
	loadfile_returns data;
	char * config  = loadfile(string,bootservices_table,&data);

	//you parse the thing and load drivers from under /drivers
	string[0] = 0x2F64726976657273;	//"/drivers"
	for (ustd_t k = 0; data->file[k] != '\n'; ++k){				//DANGER if eval is before increase...
		char * drivername = string+8;
		ustd_t j;
		for (j = 0; ((drivername[j] != '\n')&&(drivername[j] != '#'); ++j){
			drivername[j] = data->file[k];
		}
		drivername[j+1] = 0;
		loadfile_returns temp;
		loadfile(string,bootservices_table,&temp);

		Virtual_fs * vfs = get_vfs_object(void);
		Storage * file = vfs->ramcontents_to_description(temp->file,temp->length);	//dont remove length even if it is input only, scaling
		modinsert((file-vfs->pool)/sizeof(File));
		if (data->file[k] != '\n'){ while (data->file[k] != '\n'){ ++k};}
	}
}
