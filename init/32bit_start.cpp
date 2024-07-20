//NOTE compile -m32

void 32bit_start(void * imagehandle, void * efisystab){		//DANGER gcc pussy clobbers might mess the cpu mode up (should be fine though because control register changes shouldnt take effect until the end of the instruction pipeline, the swithces have to become macros...)
	auto * func = &64bit_start;
	mswitch_32to64(void);
	__asm__(
		"JMP %0\n\t"
		::"r"(func):);
}
