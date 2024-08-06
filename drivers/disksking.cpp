/*
	IN THE KERNEL'S DISKSKING SINGLETON THESE ARE THE "PROBLEMS", in order:
disksking is subsystem number 0, aka the fixed first module
none of the bottom levels have problems aside from reading and writing to disk, which is ioctl shit, not module_ctl shit.
*/
enum disksking_problems{ PARTTABLES_ID,PARTITION_TABLES,DISKS,FILESYSTEMS,};
/*
parttable:
		method 3: identify_partition_table(sector * first_sector)
*/
/*
These are the functions a disk driver must expose:
	the format for data must be the following packet, from the first to the last index:
	64bit pointer to the position on disk, 64bit pointer to memory, 64 bit number of bytes

	aids:
		The first 3 methods...
		Method 3 is read(ulong_t destination, void * source, ulong_t length).
		Method 4 is write(ulong_t destination, void * source, ulong_t length).
		from here on out..
	problems:
		---
*/
enum disk_actions{ READ=3,WRITE};
/*
lala Partition table drivers:
signature: "PART"
	aids:
		3...
		Method 3 is load_partition_table(Partition_table * target, ustd_t diskdev_findex)		which is going to convert the partition										to the kernel standard, much like file.meta

	problems:
		---
*/
enum partition_table_actions{ LOAD=3,};
/*
Filesystem drivers:
signature: "FILE"
	aids:
		First 3 yadda yadda
		Method 3 is write(Storage * file, void * source, ulong_t amount)
		Method 4 is read(Storage * file, void * destination, ulong_t amount)
		Method 5 is mkfile(Storage * proto)			makes a file on disk and fills the passed structure with ondisk info
		Method 6 is rmfile(Storage * file)
		Method 7 is mkdir(Directory * proto)
		Method 8 is rmdir(Directory * dir)
		Method 9 is filesync(File * proto)			copies everything from vfs into the disk (includes mkfile)
		Method 10 is load_file(void * ondisk, File * ret)	this needs to be able to load both directories and files and fill the type
		Method 11 is get_dir_entries(Directory * dir)		fills directory with pointers
		from here on out...
*/
enum filesystem_actions{ WRITE=3,READ,MKFILE,RMFILE,MKDIR,RMDIR,FILESYNC,LOAD_FILE,GET_DIR_ENTRIES,};




class partition{		//no pointer to the tracking of blocks because i am assuming that this is filesystem dependant
	ulong_t signature;	//"SWAP\0\0\0\0" for swapparts, a null signature is illegal
	File * disk;
	void * start;
	ulong_t length;		//total, in bytes.
	ulong_t files_number;	//includes directories, should be standard among filesystems i think?
};
class DisksKing : Driver{
	using enum disksking_problems;
	King partitions{
		partition * pool : King.pool;		//config...
	};
	void load_partition_table(Device * disk){
		void * second_sector = kshmalloc(1);	//gpt identification sector
		memset(second_sector,0,4096);
		ulong_t * diskname = &disk->name;
		dking->read(diskname*,512,second_sector,512);

		ustd_t id = problem_ctl(PARTTABLES_ID,0,0,second_sector);
		kshmfree(second_sector,1);

		partition * data = kshmalloc(512);
		memset(data,0,512*4096);
		problem_ctl(PARTITION_TABLES,id, partition_table_actions::LOAD,data);
		for (ustd_t t = 0; t < (4096*512)/sizeof(partition); ++t){
			if !(data[t]->signature){
				break;}
			memcpy(this->partitions->pool_alloc(1),&data[t],sizeof(partition));
		}
		kshmfree(data,512);
	}
	/*
	The following functions on error simply fail and return the layer of recursion the error was met on (you should mount disks manually...)
	*/

	ustd_t disk_request(ustd_t action, ustd_t partition, ulong_t reloff, void * destination, ulong_t bytes_number){
		Virtual_fs * vfs = get_vfs_object(NUH);
		Partition * parte = &this->partitions->pool[partition];
		Device * dev = parte->disk;

		void * base = destination;
		ustd_t pagetype;
		Process * process = get_process_object(NUH);
		ulong_t entry = vmto_entry(process->pagetree,base,&pagetype);
		ustd_t atom = get_multi_from_pagetype(pagetype);
		ustd_t repetitions = pagetype_roundup(bytes_number,pagetype);
		for (ustd_t g = 0; g < repetitions; 0){
			ustd_t l = 0;
			for (1; l < repetitions; ++l){
				if !(entry[g+1] == entry[g]+atom){
					break;}
			}
			bytes_number = l*atom;
			destination = base+g*atom;
			g += l;

			uint64_t * data = kshmalloc(1);
			data[0] = parte->diskpos + reloff;
			data[1] = mem_map(dev->d->thread->father->pagetree,destination,pagetype,l,cache.WRITEBACK);
			data[2] = atom;

			Threadsking * tking = get_threadsking_object(NUH);
			Thread * kernel = INIT_KERNEL_THREAD(kernel,tking);	//see ioctl

			dev->d->thread->caller = &disksking_read_called_whore;
			ustd_t fd = dev->d->runtime->children->pool[g]-vfs->descriptions)/sizeof(File);
			ioctl(fd,action,data);
			disksking_read_called_whore:
			mem_unmap(dev->d->runtime->pagetree,data[1],l,pagetype);
			kshmfree(data,1);
			TKILL(kernel);
		}
	}
	#define read(a,b,c,d) { disk_request(disk_actions::READ,a,b,c,d)};
	#define write(a,b,c,d) { disk_request(disk_actions::WRITE,a,b,c,d)};

	void load_file(ustd_t partition, ulong_t diskpos, File * ret){
		using enum filesystem_actions;
		Virtual_fs * vfs = get_vfs_object(NUH);
		Threadsking * tking = get_threadsking_object(NUH);

		Partition * part = &this->partitions->pool[partition];

		File * bystander = kshmalloc(512);
		memset(bystander,0,4096*512);

		Thread * kernel; INIT_KERNEL_THREAD(kernel,tking);
		problem_ctl(FILESYSTEMS,this->partitions[partition]->signature,LOAD_FILE,bystander);	//getting the structure
		memcpy(ret,bystander,sizeof(File));
		mem_unmap(part->disk->d->runtime->pagetree,back,1);

		kshmfree(bystander,512);
		TKILL(kernel);
	}

	enum filesystem_actions{ CREATE=5,REMOVE,};
	void filesystem_request(Storage * proto, ustd_t action, auto * data){					//data must be smaller than a smallpage
		File * bystander = kshmalloc(1);
		memcpy(bystander,proto,sizeof(File));
		memcpy(bystander+sizeof(File),data,4096-sizeof(File)));

		Threadsking * tking = get_threadsking_object(NUH);
		Thread * kernel; INIT_KERNEL_THREAD(kernel,tking);
		problem_ctl(2,this->partitions->pool[proto->part]->signature,action,bystander);			//and this is finally scaled all the way
		memcpy(proto,bystander,sizeof(File));

		kshmfree(bystander,1);
		TKILL(kernel);
	}
	#define mkfile(a){ filesystem_request(a,file_actions::CREATE)}
	#define rmfile(a){ filesystem_request(a,file_actions::REMOVE)}
	#define mkdir(a){ filesystem_request(a,file_actions::REMOVE+2)}
	#define rmdir(a){ filesystem_request(a,file_actions::REMOVE+2)}

	#define disk_sync(a){ filesystem_request(a,filesystem_actions::FILESYNC)}
};
