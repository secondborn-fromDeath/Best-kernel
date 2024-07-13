/*
	This is a driver for intel's IO apics, this file functions as the source for information along with the
	multiprocessor specification and the implementation for the Loonix kernel

	IO_apic portion (these are pin-programming functions):
		vector assignment
		masking, delivery mode,
		delivery status and sync, trigger mode

	Local portion:
		change the task priority (so interrupts lower than it are no-care)
		signal EOI
		spurious interrupt vector
		remote read from other processors' lapics
		timer interrupts (section 6.12)

coordination is the duty of callers obviously.

page 858 of the i486 manual
*/




	/*
		IO apic portion of things:
	*/

Class IO_apic{
	void * mmio;
	ustd_t pinsnum;
};

	//you can get the information on the ir pins of devices from pci
	ustd_t assign_vector(IO_apic * info, ustd_t rel_pin, uchar_t vector){
		if !(info->pinsnum > rel_pin){ return 1;}
		ustd_t * ioapic_cfg = info->mmio;		//the "default" is FEC0000.
		//writing to regsel the pin number
		ioapic_cfg[0] = 16+rel_pin;
		//writing with preserve to win
		ustd_t nyu = ioapic_cfg[1];
		(uchar_t)nyu = vector;
		ioapic_cfg[1] = nyu;
		return 0;
	}
	ustd_t get_vector(IO_apic * info, ustd_t rel_pin){
		if !(info->pinsnum > rel_pin){ return 1;}
		ustd_t * ioapic_cfg = info->mmio;		//FEC00000 + 4096*ioapic_id
		ioapic_cfg[0] = 16+rel_pin;
		ustd_t nyu = ioapic_cfg[1];
		return (char)nyu;
	}
	ustd_t mask_pin(IO_apic * info, ustd_t rel_pin){
		if !(info->pinsnum > rel_pin){ return 1;}
		ustd_t * ioapic_cfg = info->mmio;
		//pinnum into regsel
		ioapic_cfg[0] = rel_pin;
		//writing with preserve to win
		ustd_t nyu = ioapic_cfg[1];
		(uchar_t)nyu |= 1<<16;		//maskbit
		ioapic_cfg[1] = nyu;
		return 0;
	}
	ustd_t unmask_pin(IO_apic * info, ustd_t rel_pin){
		if !(info->pinsnum > rel_pin){ return 1;}
		ustd_t * ioapic_cfg = info->mmio;
		//pinnum into regsel
		ioapic_cfg[0] = rel_pin;
		//writing with preserve to win
		ustd_t nyu = ioapic_cfg[1];
		(uchar_t)nyu = (nyu | 1<<16)^1<<16;;
		ioapic_cfg[1] = nyu;
		return 0;
	}
	enum delivmod{ FIXED,LOWPRIO,RESONE,RESTWO,NMI,RESET,RESTHREE,EXTINT};
	ustd_t set_delivery_mode(IO_apic * info, ustd_t rel_pin, ustd_t mode){
		if (mode > delivmod.EXTINT){ return 1;}
		if !(info->pinsnum > rel_pin){ return 1;}
		//you know the drill
		ustd_t * ioapic_cfg = info->mmio;
		ioapic_cfg[0] = 16+rel_pin;
		ustd_t nyu = ioapic_cfg[1];
		nyu = ((nyu | 7<<8)^7<<8) | mode;	//10/10 readability
		ioapic_cfg)[1] = nyu;
		return 0;
	}
	ustd_t get_delivery_mode(IO_apic * info, ustd_t rel_pin){
		if !(info->pinsnum > rel_pin){ return 32;}
		ustd_t * ioapic_cfg = info->mmio;
		ioapic_cfg[0] = 16+rel_pin;
		ustd_t nyu = ioapic_cfg[1];
		return nyu<<16>>29;
	}
	/*
		The get_assertion_status() function might be useful is some contexts but if it is
		about blocking i want to just do it in here so i am going to wait until i find the use-case.
		Same goes for the remote IRR
	*/
	enum trigmode{ EDGE,LEVEL};
	ustd_t set_trigger_mode(IO_apic * info, ustd_t rel_pin, ustd_t trigger_mode){
		if (trigger_mode > trigmode.LEVEL){ return 1;}
		if !(info->pinsnum > rel_pin){ return 1;}
		ustd_t * ioapic_cfg = info->mmio;
		ioapic_cfg[0] = 16+rel_pin;
		ustd_t nyu = ioapic_cfg[1];
		return nyu<<;
	}
	ustd_t get_trigger_mode(IO_apic * info, ustd_t rel_pin){
		if (trigger_mode > trigmode.LEVEL){ return 1;}
		if !(info->pinsnum > rel_pin){ return 1;}
		ustd_t * ioapic_cfg = info->mmio;
		ioapic_cfg[0] = 16+rel_pin;
		ustd_t nyu = ioapic_cfg[1];
		return nyu<<15>>31;
	}



	/*
		Local apic portion of things:

	Disclaimer that this only works on 64bit processors
	*/
	ustd_t set_task_priority(ustd_t prio){
		if (prio > 15){ return 1;}
		ustd_t * lapic = 0xFEE00000;
		lapic[0x80/4] = prio;
		return 0;
	}
	//no need for get
	void signal_EOI(){
		((ustd_t *)0xFEE000B0)* = 1;
	}
	ustd_t set_spurious(uchar_t newvec){
		((ustd_t *)0xFEE000F0)* = (ustd_t)newvec;
		return 0;
	}
	ustd_t get_spurious(){
		return ((ustd_t *)0xFEE000F0)*;
	}
	/*
	You use command reg and read from remote read reg
	destination field is the *local* unit and the vector is the register
	*/
	ustd_t remote_read(Kontrol * status, ustd_t remote_id, ustd_t regnum){
		ustd_t * lapic = 0xFEE00000;
		switch (status->lapic_version){
			case LEGACY:{
				//destination
				lapic[0x300/4] = remote_id<<56;
				//vector
				ustd_t g = lapic[0x310/4];
				(char)g = regnum;
				lapic[0x310/4] = g;
			break;}
			case X2APIC:{
				ustd_t a;
				ustd_t c = 0x830;
				ustd_t dx;
				__asm__ volatile(
				"rdmsr"
				"or %4,%%eax\n\t"	//i havent read the C abi idk what that clobbers to
				"mov %%ebx,%%edx\n\t"
				"shl $32,%%edx\n\t"	//DANGER might not be 32bit
				"wrmsr"
				::"r"(a),"r"(c),"r"(dx),"r"(remote_id),"r"(regnum)
				);
			break;}
		}
		return lapic[0xC0/4];
	}
	/*
	This one might want more options but eh
	*/
	enum divider{ ONE,TWO,FOUR,EIGHT,SIXTEEN};
	void schedule_timed_interrupt(ustd_t ticks, ustd_t divider){	//reminder you dont need a time conversion lol
		ustd_t * lapic = 0xFEE00000;
		//writing to initial count
		lapic[0x380/4] = ticks;
		ustd_t pipe = 0;
		if (divider){
			//setting divide base
			lapic[0x3E0/4] = divider;	//DANGER not using the enum is undefine behaviour. i wonder if there is some compiler nonsense to enforce the passing of immediate values...
			//indicating we are going to use the divider
			pipe |= 2<<18;
		}
		lapic[0x320/4] = pipe;
	}
