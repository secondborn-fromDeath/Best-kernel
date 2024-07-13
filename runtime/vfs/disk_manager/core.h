#define ondisk_t void *
#define fs_t ulong_t



/*
What does a disk do? You read from it and write to it.

		.Class DisksKing which has an array of disks that is re-sizeable, with the methods 'read' and 'write' that call that same method from
			the argument disk's driver
		.Class Virtual_fs should contain a "scan" method which scans partition layout systems and an array of driver Classes
			deeper than that there should be the "insertion" method and another array for the filesystem drivers, which are Classes of
			methods to parse the information in the style of the filesystems are return it in the style of the vfs
		.Swapping is dealt with at the file level
*/

Class SwapKing;
Class DisksKing{
	void read(ustd_t disk, void * disk_offset, void * buf, ustd_t pages_number, ustd_t pagetype);
	void write(ustd_t disk, void * disk_offset, void * buf, ustd_t pages_number, ustd_t pagetype);
	Device ** disks;
	ustd_t disks_number;
	ustd_t swaps_array[32];	//indexes into vfs
};
Class Disk_driver{
	void read(void);	//wont stay void
	void write(void);
	ulong_t getsize(void);
	//power management function pointers???
};
Class Filesystem_driver : Directory{
	ulong_t recurse_directories(char_t * path, ulong_t dir, ulong_t * stand){	//read runtime/vfs/core.h for reference
	} : File.recurse_directories;
	void constructor(File * where);
	void del(void);
	void chmeta(void);
};
