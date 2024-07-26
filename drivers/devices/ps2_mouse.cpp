/*
This is a driver for the ps2 mouse

the windowing system runs as pid 1, it runs with owner_ids.ROOT so that it cant
connect to the internet
*/

//interrupt routine
volatile void ps2_mouse_get_packet(void){
	uint8_t info = ps2_in(void);
	signed int8_t x = ps2_in(void);
	signed int8_t y = ps2_in(void);
	ps2_disable_device(MOUSE);
	ps2_disable_device(KEYBOARD);	//these will be activated when the framebuffer is written to

	ProcessKing * prcking = get_processking_object(void);
	Thread * thread = &prcking->pool[1]->children[0];
	thread->sigset &= MAX32BIT^SIGPOLLING;
	File * mousefil = &get_vfs_object(void)->pool[thread->parent->descs[thread->poll[MOUSE]->fdesc]->findex];
	mouseinfo * pipe = mousefil->shared_contents[0];

	pipe* ={
		.x = x + ((info & 16)<<8);		//words, not bytes
		.xaxis_overflow = (info & 64);
		.yaxis_overflow = (info & 128)
		.y = y + + ((info & 32)<<8);
		.left_click = info & 1;
		.right_click = info & 2;
		.middle_click = info & 3;
	};
}
//sensitivity multiplier??? i cant find information anywhere
volatile void ps2_mouse_set_scaling(ustd_t scale){
	ps2_select_device(MOUSE);
	outb(0xE6+scale,0x64);		//two opcodes for 1:1 and 2:1
	flush_OBF(void);
	ps2_enable_device(KEYBOARD);
}
//movement precision, in count per millimeter, enum argument, powers of two
volatile ps2_mouse_set_resolution(ustd_t res){
	ps2_select_device(MOUSE);
	outb(0xE8,0x64);
	outb(res,0x60);
	flush_OBF(void);
	ps2_enable_device(KEYBOARD);
}
