/*
This is a full driver for both the keyboard and mouse

NOTE:
	this is a default driver
*/


/*
This is the embedded controller side of things*/
namespace{
	enum{ KEYBOARD,MOUSE,};
};
volatile uint8_t ps2_in(void){
	while !(inb(0x64) & 1);
	return inb(0x60);
}
volatile uint8_t ps2_read_conf(void){
	outb(0x20,0x64);
	flush_OBF(void);		//flushing the acknowledge
	return ps2_in(void);		//returning the actual data
}
volatile void ps2_write_conf(uint8_t nyu){
	outb(0x60,0x64);		//writing to the configuration byte
	outb(nyu,0x60);
	flush_OBF(void);
}
volatile enable_A20(void){
	outb(0xD0,0x64);		//cmd: Read from port 2 ("output port")
	flush_OBF(void);
	uint8_t conf = ps_in(void);
	if !(conf & 2){
		ps2_write_conf(conf|2);
	}
}
volatile POST_check(void){
	uint8_t conf = ps2_read_conf(void);
	if !(conf & 4){
		shutdown(void);
	}
}
volatile ustd_t ps2_probe_device(ustd_t dev){
	uint8_t conf = ps2_read_conf(void);
	return conf & (16<<dev);		//returning NULL if no device
}
volatile void ps2_disable_device(ustd_t dev){
	outb(0xAD - dev*13);
	flush_OBF(void);
}
volatile void ps2_enable_device(ustd_t dev){
	outb(0xAE - dev*13);
	flush_OBF(void);
}
volatile void ps2_select_device(ustd_t dev){
	ps2_disable_device(!dev);
	ps2_enable_device(dev);
}
volatile void ps2_reset_device(ustd_t dev){
	ps2_select_device(dev);
	outb(0xFF,0x64);			//slave reset command
	flush_OBF(void);
	if (ps2_in(void) != 0xAA){
		shutdown(void);			//assuming the worst
	}
}
volatile void ps2_flush_OBF(void){
	while !(inb(0x64) & 1);
	inb(0x60);
}
void ps2_create_device_structures(void){
	Kontrol * ctrl = get_kontrol_object(void);
	Virtual_fs * vfs = get_vfs_object(void);
	for (ustd_t i = 0; i < 2; ++i){
		if (ps2_probe_device(i)){
			Device * newdev = vfs->pool_alloc(1);
			Directory * devices = &vfs->descriptions[ctrl->devices_directory];
			devices->pool = get_kingpointer_object()->pool_realloc(devices->pool,devices->length,devices->length+1);
			++devices->length;
			devices->pool[devices->length] = newdev;
			memset(newdev,0,sizeof(Device));
			switch (dev){
				case KEYBOARD:{
					newdev->irline = 12;
				}
				case MOUSE:{
					newdev->irline = 1;
				}
			}
		}
	}
}

volatile void ps2ctrl_init(void){
	ps2_disable_device(KEYBOARD);
	ps2_disable_device(MOUSE);

	ps2_flush_OBF(void);

	if (ps2_probe_device(KEYBOARD)){ ps2_reset_device(KEYBOARD);}
	if (ps2_probe_device(MOUSE)){ ps2_reset_device(MOUSE);}

	ps2_create_device_structures(void);
}
