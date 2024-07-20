/*
The drivers subsystems is structured so that any given driver only has to comply with the father it is being loaded for
Every subssytem stems from either a kernel management subsystem or HARDWARE (meaning that if you want to fight the NSA with strange nu-internet protocols you can do it, through IOCTL, write your own specification and see devices_core.h)

Driver executable files have an executable header (see SYSEXEC.cpp) and the drivercode header
*/

struct drivercode{
	ustd_t additional_data[2];	//in devices the classcodes and models files
	ustd_t typerec;			//recursion count for ins,rm
	ustd_t type[];
	ustd_t methods_num;			//recursion count for ins,rm
	auto * functions[];
};
Class drivQuirk{			//problem with array of solutions
	fiDriv ** pool : King.pool;
};
Class QuirksGod : King{
	drivQuirk * pool : King.pool;
};
Class Driver : King{			//protocol with an array of problems
	drivQuirk ** pool : King.pool;
	Process * runtime;
	fiDriv ** double_link;
};
Class fiDriv : File;	//includes Driver
Class DriversGod : King{
	fiDriv ** pool : King.pool;
};
