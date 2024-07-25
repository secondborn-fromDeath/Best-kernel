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
	Kontrol * ctrl = get_kontrol_object(void);
	struct rtc_time rtcret;
	uefi_wrapper(ctrl->efifuncs.getrtc,&rtcret,0,0,0,0,0);
	return rtcret->hour*3600*1000000000 + rtcret->minute*60*10000000000 + rtcret->second*1000000000 + rtcret->nanosecond;
}

void get_timespec(struct rtc_time * output){
	Kontrol * ctrl = get_kontrol_object(void);
	uefi_wrapper(ctrl->efifuncs.getrtc,output,0,0,0,0,0);
}
ustd_t set_timespec(struct rtc_time * rtcinsert){	//root things
	if (get_process_object(void)->owner_id != users.ROOT){ return 1;}
	Kontrol * ctrl = get_kontrol_object(void);
	uefi_wrapper(ctrl->efifuncs.setrc,rtcinsert,0,0,0,0,0);
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
	Kontrol * ctrl = get_kontrol_object(void);
	struct EFIwakeup_time wkup_ret;
	uefi_wrapper(ctrl->efifuncs.getrtc,wkup_ret,0,0,0,0,0);
}

void set_wakeup(struct rtc_time * wkup_insert){
	Kontrol * ctrl = get_kontrol_object(void);
	uefi_wrapper(ctrl->efifuncs.set_wakeup,1,identity,0,0,0,0);
}
