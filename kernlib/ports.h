void outb(uint8_t data, uint16_t port){
	__asm__(
	"MOVq	%%ecx,%%edx\n\t"
	"OUTb	%%al,%%dx\n\t"
	::"r"(data),"r"(port):);
}
void outw(uint16_t data, uint16_t port){
	__asm__(
	"MOVq	%%ecx,%%edx\n\t"
	"OUTw	%%ax,%%dx\n\t"
	::"r"(data),"r"(port):);
}
void outd(uint32_t data, uint16_t port){
	__asm__(
	"MOVq	%%ecx,%%edx\n\t"
	"OUTd	%%eax,%%dx\n\t"
	::"r"(data),"r"(port):);
}

uint8_t inb(uint16_t port){
	uint8_t data;
	__asm__(
	"MOVq	%%eax,%%edx\n\t"
	"INb	%%dx,%%al\n\t"
	:"=r"(data):"r"(port):);
	return data;
}
uint16_t inw(uint16_t port){
	uint16_t data;
	__asm__(
	"MOVq	%%eax,%%edx\n\t"
	"INw	%%dx,%%ax\n\t"
	:"=r"(data):"r"(port):);
	return data;
}
uint32_t ind(uint16_t port){
	uint32_t data;
	__asm__(
	"MOVq	%%eax,%%edx\n\t"
	"INb	%%dx,%%eax\n\t"
	:"=r"(data):"r"(port):);
	return data;
}



