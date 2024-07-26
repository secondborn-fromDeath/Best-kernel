/*
This is a driver for the PS2 keyboard
*/

namespace { SCOLL,NUMBER,CAPS,};
volatile void ps2_keyboard_LEDS(ustd_t which){
	ps2_device_select(KEYBOARD);
	outb(0xED,0x64);
	outb(which,0x60);
	flush_OBF(void);

	ps2_device_enable(MOUSE);
}
#define ps2_keyboard_set_scrolllock(void){ ps2_keyboard_LEDS(SCROLL)}
#define ps2_keyboard_set_numberlock(void){ ps2_keyboard_LEDS(NUMBER)}
#define ps2_keyboard_set_capslock(void){ ps2_keyboard_LEDS(CAPS)}

//this is the only scancode i support
volatile void ps2_keyboard_set_scancode_1(void){
	ps2_device_select(KEYBOARD);
	outb(0xF0,0x64);
	outb(0x1,0x60);
	flush_OBF(void);

	ps2_enable_device(MOUSE);
}

namespace keyb_sensitivity{ MS250,MS500,MS750,MS1000,};
volatile void ps2_keyboard_set_keyboard_sensitivity(ustd_t sens){
	ps2_device_select(KEYBOARD);
	outb(0xF3,0x64);
	outb(0x0 | (sens<<5),0x60);
	flush_OBF(void);

	ps2_device_enable(MOUSE);
}

//interrupt routine
volatile void ps3_keyboard_get_char(void){
	uint8_t code = ps2_in(void);
	ps2_disable_device(KEYBOARD);
	ps2_disable_device(MOUSE);

	ProcessKing * prcking = get_processking_object(void);
	Thread * thread = &prcking->pool[1]->children[0];
	thread->sigset &= MAX32BIT^SIGPOLLING;
	thread->poll[KEYBOARD]->retaction = WRITE;
	File * keybfil = &get_vfs_object(void)->pool[thread->parent->descs[thread->poll[KEYBOARD]->fdesc]->findex];
	keybinfo * pipe = keybfil->shared_contents[0];

	pipe->code = code;
}
