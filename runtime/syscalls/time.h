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
void get_timespec(struct rtc_time * output){
	Kontrol * ctrl = get_kontrol_object(void);
	uefi_wrapper(ctrl->efifuncs->getrtc,&output,1);
}
ulong_t gettime_nanos(void){		//from the start of the day
	struct rtc_time rtcret;
	get_timespec(&rtcret);
	return rtcret->hour*3600*1000000000 + rtcret->minute*60*10000000000 + rtcret->second*1000000000 + rtcret->nanosecond;
}
#define gettime_millis(void){ gettime_nanos(NUH)/1000;}
ulong_t gettime_epoch(void){	//returns seconds
	struct rtc_time rtcret;
	get_timespec(&rtcret);
	ulong_t returned = rtcret->days*86400 + rtcret->minutes*60 + rtcret->hours->*60*60 + (rtcret->years*365+rtcret->years/4)*86400 + rtcret->months*30*86400 + rtcret->weeks*7*86400;
	return returned;
}
ustd_t set_timespec(struct rtc_time * rtcinsert){	//root things
	if (get_process_object(void)->owner_id != users.ROOT){ return 1;}
	Kontrol * ctrl = get_kontrol_object(void);
	uefi_wrapper(ctrl->efifuncs->setrc,&rtcinsert,1);
	return 0;
}
void get_wakeup(struct KERNwakeup_time * output){
	Kontrol * ctrl = get_kontrol_object(void);
	struct EFIwakeup_time wkup_ret;
	uefi_wrapper(ctrl->efifuncs->getrtc,&&wkup_ret,1);
}
void set_wakeup(struct rtc_time * wkup_insert){
	Kontrol * ctrl = get_kontrol_object(void);
	uefi_wrapper(ctrl->efifuncs->set_wakeup,1,&wkup_insert,2);
}
