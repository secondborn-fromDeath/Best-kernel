/*
This enumerates the devices, then loads the drivers and then enumerates the devices again (updating state in kontrol)

devices can be loaded without a driver but drivers cant be loaded without devices, the attachment of devices to drivers is done in
the devices function so this is what you get
*/

void enact_IO_context(void){
	Kontrol * ctrl = get_kontrol_object(void);
	Virtual_fs * vfs = get_vfs_object(void);
	ctrl->dev_directory = (vfs->descriptions_alloc(1)-vfs->descriptions)/sizeof(vfs->descriptions*);	//implied mutex because there is only one processor up so...

	enumerate_devices(void);
	setup_drivers(void);
	enumerate_devices(void);
}
