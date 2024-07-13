struct rtc_time{
	struct timespec{
		uint16_t year;
		uint8_t month;
		uint8_t day;
		uint8_t hour;
		uint8_t minute;
		uint8_t second;
		uint8_t pad1;
		uint32_t nanosecond;
	}; sec;
	signed int16_t timezone;	//-1440 through +1440, 2047 indicates an unknown timezone
	uint8_t daylight;
	uint8_t pad2;
};

ulong_t gettime_nanos(void){		//from the start of the day
	Kontrol * ctrl; get_kontrol_object(ctrl);
	struct rtc_time * rtcret = (ctrl->efifuncs.getrtc)(void);
	return rtcret->hour*3600*1000000000 + rtcret->minute*60*10000000000 + rtcret->second*1000000000 + rtcret->nanosecond;
}

void get_timespec(struct rtc_time * output){
	Kontrol * ctrl; get_kontrol_object(ctrl);
	struct rtc_time * rtcret = (ctrl->efifuncs.getrtc)(void);
	Kingmem * mm; get_kingmem_obejct(mm);
	Process * process = get_process_object(void);
	void * backup = mm->vm_ram_table;
	mm->vm_ram_table = process->pagetree;
	void * identity = mm->vmto_phys(output);
	memcpy(identity,rtcret,sizeof(struct rtc_time));
	mm->vm_ram_table = backup;
}

ustd_t set_timespec(struct rtc_time * rtcinsert){	//root things
	Process * process = get_process_object();
	if (process->owner_id != users.ROOT){ return 1;}
	Kingmem * mm; get_kingmem_object(mm);
	void *backup = mm->vm_ram_table;
	mm->vm_ram_table = process->pagetree;
	void ins = vmto_phys(rtcinsert);
	Kontrol * ctrl; get_kontrol_object(ctrl);
	ustd_t a;
	__asm__(
	"call %%rcx\n\t"				//NOTE bios porting
	::"r"(ins),"r"(ctrl->efifuncs.setrtc):
	);
	return 0;
}

struct EFIwakeup_time{
	bool * enabled;
	bool * pending;
	struct rtc_time * wkup_spec;
};
struct KERNwakeup_time{
	struct rtc_time time;
	bool enabled;
	bool pending;
};

void get_wakeup(struct KERNwakeup_time * output){
	Kontrol * ctrl; get_kontrol_object(ctrl);
	struct EFIwakeup_time * wkup_ret = (ctrl->efifuncs.getrtc)(void);
	Kingmem * mm; get_kingmem_obejct(mm);
	Process * process = get_process_object(void);
	void * backup = mm->vm_ram_table;
	mm->vm_ram_table = process->pagetree;
	struct KERNwakeup_time * identity = mm->vmto_phys(output);
	memcpy(&identity->time,wkup_ret,sizeof(struct rtc_time));
	identity->enabled = wkup_ret->enabled*;
	identity->pending = wkup_ret->pending*;
	mm->vm_ram_table = backup;
}

void set_wakeup(struct rtc_time * wkup_insert){
	Kontrol * ctrl; get_kontrol_object(ctrl);
	Kingmem * mm; get_kingmem_obejct(mm);
	Process * process = get_process_object(void);
	void * backup = mm->vm_ram_table;
	mm->vm_ram_table = process->pagetree;
	struct rtc_time * identity = mm->vmto_phys(wkup_insert);
	bool a = true;
	__asm__(
	"call %%rdx\n\t"
	::"r"(a),"r"(identity),"r"(ctrl->efifuncs.set_wakeup):
	);
}
