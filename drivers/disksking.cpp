/*
	IN THE KERNEL'S DISKSKING SINGLETON THESE ARE THE "PROBLEMS", in order:
disksking is subsystem number 0, aka the fixed first module
none of the bottom levels have problems aside from reading and writing to disk, which is ioctl shit, not module_ctl shit.



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

lala Partition table drivers:
signature: "PART"
	aids:
		3...
		Method 3 is load_partition_table(Partition_table * target, ustd_t diskdev_findex)		which is going to convert the partition
										to the kernel standard, much like file.meta
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
		Method 8 is filesync(File * proto)			copies everything from vfs into the disk (includes mkfile)
		Method 10 is load_file(void * ondisk, File * ret)	this needs to be able to load both directories and files and fill the type
									when loading a directory it should be filled with the children pointers
									which you may allocate wherever in the page you are given
		from here on out...

see disksking for the in-kernel "driver"
*/

class Disk_driver : fiDriv;
class Fs_driver : fiDriv;		//the peasant-type is variable by blocksize, we pass teh size of the type explicitly to pool-FUNCTION anyway though
class Disk : Device{
	Fs_driver ** pool : Device.pool;
	ustd_t parts_number : Device.length;
	Disk_driver * driver : Device.driver;
};
class partition{		//no pointer to the tracking of blocks because i am assuming that this is filesystem dependant
	ulong_t signature;	//"SWAP\0\0\0\0" for swapparts, a null signature is illegal
	ustd_t disk;
	void * start;
	ulong_t length;		//total, in bytes.
	ulong_t files_number;	//includes directories, should be standard among filesystems i think?
};
class DisksKing : Driver{
	King partitions{
		partition * pool : King.pool;		//config...
	};
	ustd_t init(void){	//scans for partitions on disk devices
		Directory * dev = get_vfs_obejct(void)->descriptions[1];
		for (ustd_t g = 0; g < dev->chidlren->length; ++g){
			if (dev->children->pool[g]->signature == 0x4449534B){		//in ascii "DISK"
				partition * data = malloc(1,pag.MIDPAGE);
				memset(data,0,512*4096);
				problem_ctl(0,0,this->partitions);			//MASSIVE ATTACK VECTOR!!!
				for (ustd_t t = 0; t < (4096*512)/sizeof(partition); ++t){
					if !(data[t]->signature){
						break;}
					memcpy(this->partitions->pool_alloc(1),&data[t],sizeof(partition));
				}
			}
		}
	}
	/*
	The following functions on error simply fail and return the layer of recursion the error was met on (you should mount disks manually...)

	DANGER NOTE NOTE here it is assumed that the disk can do DMA to whatever addresses it wants and there is no bus-wise limitation.
	*/

	enum disk_actions{ READ = 3,WRITE,};
	ustd_t disk_request(ustd_t action, ulong_t diskname, ustd_t diskpos, volatile void * destination, volatile ulong_t bytes_number){
		Virtual_fs * vfs = get_vfs_object(void);
		Directory * dev = &vfs->descriptions[1];
		for (ustd_t g = 0; g < dev->children->length; ++g){
			if (dev->children->pool[g]->type == 0x4449534B){
				if (dev->children->pool[g]->name == diskname){
					void * base = destination;
					ustd_t pagetype;
					ulong_t entry = vmto_entry(get_process_object(void)->pagetree,base,&pagetype);
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

						void * back = mem_map(dev->d->thread->father->pagetree,destination,pagetype,l,cache.WRITEBACK);
						Threadsking * tking = get_threadsking_object(void);
						Thread * kernel = INIT_KERNEL_THREAD(kernel,tking);	//see ioctl

						dev->d->thread->caller = &disksking_read_called_whore;
						ioctl((dev->d->runtime->children->pool[g]-vfs->descriptions)/sizeof(File),action,&diskpos);
						disksking_read_called_whore:
						mem_unmap(dev->d->runtime->pagetree,back,l,pagetype);
						tking->free(kernel,1);
					}
				}
			}
		}
	}
	#define read(a,b,c,d) { disk_request(disk_actions::READ,a,b,c,d)};
	#define write(a,b,c,d) { disk_request(disk_actions::WRITE,a,b,c,d)};

	void load_file(ustd_t partition, ulong_t diskpos, File * ret){
		Virtual_fs * vfs = get_vfs_object(void);
		File * bystander = malloc(1,pag.MIDPAGE)
		memset(bystander,0,4096*512);
		Threadsking * tking = get_threadsking_object(void);
		void * back = mem_map(this->pool[2]->runtime->pagetree,bystander,1,pag.MIDPAGE,cache.WRITEBACK);
		Thread * kernel; INIT_KERNEL_THREAD(kernel,tking);
		problem_ctl(2,this->partitions[partition]->signature,10,bystander);		//and this is finally scaled all the way
		memcpy(ret,bystander,sizeof(File));
		if (ret->type == directory){
			vfs->pool_alloc(ret->children->count);
			problem_ctl(2,this->partitions[partition]->signature,9,bystander);		//and this is finally scaled all the way
		}
		mem_unmap(this->pool[2]->runtime->pagetree,back,1);
		free(bystander,pag.MIDPAGE);
		tking->pool_free(kernel,1);
	}

	enum filesystem_actions{ CREATE=5,REMOVE,};
	void filesystem_request(Storage * proto, ustd_t action){
		File * bystander = malloc(1,pag.MIDPAGE)
		memset(bystander,0,4096*512);
		memcpy(bystander,proto,sizeof(File));
		void * back = mem_map(this->pool[2]->runtime->pagetree,bystander,1,pag.MIDPAGE,cache.WRITEBACK);
		Threadsking * tking = get_threadsking_object(void);
		Thread * kernel; INIT_KERNEL_THREAD(kernel,tking);
		problem_ctl(2,this->partitions[proto->part]->signature,action,bystander);			//and this is finally scaled all the way
		memcpy(proto,bystander,sizeof(File));
		mem_unmap(this->pool[2]->runtime->pagetree,back,1);
		free(bystander,pag.MIDPAGE);
		tking->pool_free(kernel,1);
	}
	#define mkfile(a){ file_request(a,file_actions::CREATE)}
	#define rmfile(a){ file_request(a,file_actions::REMOVE)}
	#define mkdir(a){ file_request(a,file_actions::REMOVE+2)}
	#define rmdir(a){ file_request(a,file_actions::REMOVE+2)}

	#define disk_sync(a){ filesystem_request(a,8)}
};
