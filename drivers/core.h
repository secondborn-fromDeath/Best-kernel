/*
All drivers, device or non should support the methods check_classcode and attach_model as the first and second methods respectively
they also have to take two targuments auto * data,ret

The directory for drivers is /dev/drivers (meaning fd1/drivers)
*/

struct drivercode;	//see the loading syscalls

Class Driver : File;
Class DriversGod : King{ File ** pool : King.pool;};
Class Runtime_driver : Process{
	Driver * file;		//doubly linked because it is faster to not go through vfs on interruptions
};

#define check_classcode(x) (functions[0])(x)
#define attach_model(x) (functions[1])(x)	//this one gives the passed structure the pointer to the functions on its own, on fail returns 1
