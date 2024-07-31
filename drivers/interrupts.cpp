/*
	This is a driver for intel's IO apics, this file functions as the source for information along with the
	multiprocessor specification and the implementation for the Loonix kernel

	IOapic portion (these are pin-programming functions):
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

class IOapic{
	void * pointer;
	ustd_t linesnum;
	ustd_t global_base;
};

	//you can get the information on the ir pins of devices from pci (and acpi)
	ustd_t assign_vector(IOapic * info, ustd_t rel_line, uchar_t vector){
		ustd_t * ioapic_cfg = info->mmio;		//the "default" is FEC0000.
		//writing to regsel the pin number
		ioapic_cfg[0] = 16+rel_line;
		//writing with preserve to win
		ustd_t nyu = ioapic_cfg[1];
		(uchar_t)nyu = vector;
		ioapic_cfg[1] = nyu;
		return 0;
	}
	ustd_t get_vector(IOapic * info, ustd_t rel_pin){
		if !(info->linesnum > rel_pin){ return 1;}
		ustd_t * ioapic_cfg = info->mmio;		//FEC00000 + 4096*ioapic_id
		ioapic_cfg[0] = 16+rel_pin;
		ustd_t nyu = ioapic_cfg[1];
		return (char)nyu;
	}
	ustd_t mask_pin(IOapic * info, ustd_t rel_pin){
		if !(info->linesnum > rel_pin){ return 1;}
		ustd_t * ioapic_cfg = info->mmio;
		//pinnum into regsel
		ioapic_cfg[0] = rel_pin;
		//writing with preserve to win
		ustd_t nyu = ioapic_cfg[1];
		(uchar_t)nyu |= 1<<16;		//maskbit
		ioapic_cfg[1] = nyu;
		return 0;
	}
	ustd_t unmask_pin(IOapic * info, ustd_t rel_pin){
		if !(info->linesnum > rel_pin){ return 1;}
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
	ustd_t set_delivery_mode(IOapic * info, ustd_t rel_pin, ustd_t mode){
		if (mode > delivmod.EXTINT){ return 1;}
		if !(info->linesnum > rel_pin){ return 1;}
		//you know the drill
		ustd_t * ioapic_cfg = info->mmio;
		ioapic_cfg[0] = 16+rel_pin;
		ustd_t nyu = ioapic_cfg[1];
		nyu = ((nyu | 7<<8)^7<<8) | mode;	//10/10 readability
		ioapic_cfg)[1] = nyu;
		return 0;
	}
	ustd_t get_delivery_mode(IOapic * info, ustd_t rel_pin){
		if !(info->linesnum > rel_pin){ return 32;}
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
	ustd_t set_trigger_mode(IOapic * info, ustd_t rel_pin, ustd_t trigger_mode){
		if (trigger_mode > trigmode.LEVEL){ return 1;}
		if !(info->linesnum > rel_pin){ return 1;}
		ustd_t * ioapic_cfg = info->mmio;
		ioapic_cfg[0] = 16+rel_pin;
		ustd_t nyu = ioapic_cfg[1];
		return nyu<<;
	}
	ustd_t get_trigger_mode(IOapic * info, ustd_t rel_pin){
		if (trigger_mode > trigmode.LEVEL){ return 1;}
		if !(info->linesnum > rel_pin){ return 1;}
		ustd_t * ioapic_cfg = info->mmio;
		ioapic_cfg[0] = 16+rel_pin;
		ustd_t nyu = ioapic_cfg[1];
		return nyu<<15>>31;
	}
	ustd_t get_max_redir_entries(ustd_t * ioapic){
		ioapic[0] = 0;
		return ioapic[1]<<8>>8;				//id 0:23
	}
	/*
		Local apic portion of things:

	Disclaimer that this only works on 64bit processors
	*/
	ustd_t * get_lapic_pointer(void){
		Kontrol * ctrl = get_kontrol_object(void);
		return ctrl->lapic_override;
	}

	ustd_t set_task_priority(ustd_t prio){
		if (prio > 15){ return 1;}
		get_lapic_pointer(void)[0x80/4] = prio;
		return 0;
	}
	//no need for get
	void signal_EOI(void){
		get_lapic_pointer(void)[0xB0/4] = 1;
	}
	ustd_t set_spurious(uchar_t newvec){
		get_lapic_pointer(void)[0xF0/4] = (ustd_t)newvec;
		return 0;
	}
	ustd_t get_spurious(void){
		return get_lapic_pointer(void)[0xF0/4];
	}
	/*
	You use command reg and read from remote read reg
	destination field is the *local* unit and the vector is the register
	*/
	ustd_t remote_read(Kontrol * status, ustd_t remote_id, ustd_t regnum){
		ustd_t * lapic = get_lapic_pointer(void);
		lapic[0x310/4] = brother_id<<24;
		ustd_t interrupt_mask = get_targetipi_mask(void) | regnum;
		((char *)lapic)[0x300/8] = regnum;
		return lapic[0xC0/4];
	}
	void init_timer(void){
		ustd_t * lapic = get_lapic_pointer(void);
		ustd_t f = lapic[320/4];
		f &= 0xFFFFFFFF^(1<<17);				//configuring it as one-shot and it will stay that way forever
		(char)f = OS_INTERRUPTS::TIMER;				//assigning it the vector
		lapic[320/] = f;
	}
	void mask_timer(void){
		ustd_t * lapic = get_lapic_pointer(void);
		ustd_t f = lapic[320/4];
		f |= 1<<16;				//DANGER we need it to work like this step by step.
		lapic[320/] = f;			//setting the mask bit
	}
	void unmask_timer(void){
		ustd_t * lapic = get_lapic_pointer(void);
		ustd_t f = lapic[320/4];
		f &= 0xFFFFFFFF^(1<<16);		//clearing the mask bit
		lapic[320/] = f;
	}
	/*
	This one might want more options but eh
	*/
	enum divider{ ONE,TWO,FOUR,EIGHT,SIXTEEN};
	void schedule_timed_interrupt(ustd_t ticks, ustd_t divider){	//reminder you dont need a time conversion lol
		ustd_t * lapic = get_lapic_pointer(void);

		//writing to initial count
		lapic[0x380/4] = ticks;
		ustd_t pipe = lapic[0x320/4];
		if (divider){
			//setting divide base
			lapic[0x3E0/4] = divider;
			//indicating we are going to use the divider
			pipe |= 2<<18;
		}
		lapic[0x320/4] = pipe;
		unmask_timer(void);
	}
	ustd_t get_targetipi_mask(void){
		return 1<<14 | 1<<15 | 0<<18;		//assert, level triggered, target in +0x310
	}
	ustd_t get_selfipi_mask(void){
		return 1<<14 | 1<<15 | 1<<18;		//only self
	}
	ustd_t get_allipi_mask(void){
		return 1<<14 | 1<<15 | 2<<18;		//all
	}
	ustd_t get_exselfipi_mask(void){
		return 1<<14 | 1<<15 | 3<<18;		//all excluding self
	}
	ustd_t get_startupmode_mask(void){
		return 6<<8;
	}
	ustd_t get_fixedmode_mask(void){
		return 0;
	}
	void set_ipi_lowreg(ustd_t mask){
		ushort_t * lapic = get_lapic_pointer(void);
		lapic[0x302/2] = mask>>16;		//writing to the high word
		((char *)lapic)[0x301] = mask<<16>>24;	//writing to the second byte		low byte causes IPI to be sent...
	}

	void poke_brother(ustd_t brother_id, ustd_t action){
		ustd_t * lapic = get_lapic_pointer(void);
		lapic[0x310/4] = brother_id<<56;
		lapic[0x300/4] = get_targetipi_mask(void) | action;
	}
	#define wake_brother(brother_id){ poke_brother(brother_id,ints.WAKE)}
	#define sleep_brother(brother_id){ poke_brother(brother_id,ints.SLEEP)}

	void brothers_poke(ustd_t action){
		ustd_t * lapic = get_lapic_pointer(void);
		lapic[0x300/4] = get_exselfipi_mask(void) | action;
	}
	#define wake_brothers(void){ brothers_poke(ints.WAKE)}
	#define sleep_brother(void){ brothers_poke(ints.SLEEP)}

	#define brothers_init(void){
		 set_ipi_mode()brothers_poke()

	void family_poke(ustd_t interrupt){
		ustd_t * lapic = get_lapic_pointer(void);
		lapic[0x300/4] = get_allipi_mask(void) | interrupt;
	}

	void INT(ustd_t num){				//because in ringzero the ipi setting is undefined while in userspace it is self
		set_ipi_mode(get_selfipi_mask(void));
		__asm__("INT %%al\n\t"::"r"(num):);
	}

	void iret(void){
		__asm__("IRETQ\n\t");
	}
	void halt(void){
		set_task_priority(0);			//we want the hardware to help us out with not overworking one processor
		__asm__ volatile(
		"MOVq	(%%rip),%%rax\n\t"
		"MOVq	%%rax,%%cr12\n\t"
		"HLT\n\t"
		);
	}
	void wake(void){
		__asm__ volatile(
		"MOVq	%%cr12,%%rax\n\t"
		"JMP	%%rax\n\t"
		);
	}
