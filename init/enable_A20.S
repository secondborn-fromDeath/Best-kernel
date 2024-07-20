//NOTE this is all firmware SMI stuff, dont sweat the pin specifications

enable_A20:
OUT	0x64,0xD0	//cmd: Read from output port
IN	al,0x60		//storing result
TEST	al,0x02		//if A20 is already enabled return
JNZ	A20lazy_firmware
RET
A20lazy_firmware:
OR	al,0x02		//setting A20
OUT	0x64,0xD1	//cmd: Write to output port
OUT	0x60,al
RET
