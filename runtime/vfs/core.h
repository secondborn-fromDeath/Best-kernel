#include <conf/processing.cfg>
#include <other/filesystems.h>
#include <generic/types.h>


/*
	Everything is a file, root is fd 0, IO is fd 1, terminal in and out are 2 and 3
	There is a big pool of File objects, descriptors hold an index into this pool
	access to descriptors is done via a process-local pool to allow for polling and some userenv things...

	//you can preface all of the funcions that take an index to take a Directory pointer with some sort of switch statement

*/


enum filetypes{
	DIRECTORY,
	STORAGE,
	DEVICE,
};

Class Virtual_fs : King{
	void scan_partitions(void);	//returns pointers and ids into an array
	Partlayout_driver partlay[???];
	void insert_file(void);		//filesystem-style translator
	Filesystem_driver fs[??];

	File * pool : King.pool;
	extern get_rand32(void) : King.get_rand(void);

	/*
	All of these take indexes to directories and relative paths and return indexes to Files, userspace wrappers use the local Descriptor pool
	*/
	ulong_t recurse_directories(uchar_t * path, ulong_t dir, ustd_t * stand){
		for (ustd_t g = 0; g < this.pool[dir]->active_cnt; ++g){
			ulong_t attempt = this.pool[this.pool[dir]->children[g]];
			for (ustd_t i = 0; 1; ++i){
				if ((path[i] == '/') && (this.pool[attempt]->name[i] == 0)){
					stand[0] = attempt;	//so that i can return the last directory that was successful
					stand[1] = i+1;
					ulong_t t = recurse_directories(path[i+1],attempt);
					if (t != 0xbadfile){ return t;}
					break;	//just so something silly in fs wont affect here
				}
				if (path[i] != this.pool[attempt]->name[i]){ break;}
				if !(path[i]){ return attempt;}	//success
			}
		}
		return -1;	//fail D:
	}

	ulong_t open(uchar_t * path, ulong_t dir_index, ustd_t flags){	//can do mkfile from here, compressing because it is one check anyway and we do disksync
		void * desc_ptr = this.descriptions;
		ulong_t stand[2];
		ustd_t ret = recurse_directories(path,dir_index,stand);
		if (ret){ return ret;}
		else{
			if (flags & O_CREATE){
				File temp ={
					.meta.type = RAM_STORAGE;
					.meta.mode = 		//NOTE you have to figure this out later down the line
					.access_flag = 0;
					.contents = NULL;
					.length = 0;
				};
				for (ustd_t i = 0; path[stand[1]]; ++i){temp.name[i] = path[stand[1+i]];}
				return description_alloc(&temp);
			}
		}
		return -1;
	}

	//reading files directly from disk, if you dont like it you can go and call mmap
	ustd_t read(ulong_t index, void * buf, ulong_t amount, ulong_t offset){
		if (offset > this.pool[index].meta.length){ return 4;}
		switch (this.pool[index].type){
			case DIRECTORY:{ return 1;}
			case DEVICE:{ return 2;}
			default:{
				if (this.pool[index].contents >> 50){	//meaning if the file is not in RAM
					if !(this.pool[index].length){return 3;}
					DisksKing * dking; get_disksking_object(dking);
					dking->read(buf,this.pool[index].disk,this.pool[index].diskpos+offset,length);
				}
				else{ memcpy(buf,this.pool[index].contents+offset,amount);}
			return 0;}
		}
	}
	ustd_t write(ulong_t index, void * buf, ulong_t amount, offset){
		if (offset > this.pool[index].meta.length){ return 4;}
		switch (this.pool[index].type){
			case DIRECTORY:{ return 1;}
			case DEVICE:{ return 2;}
			default:{
				if (this.pool[index].contents >> 50){	//meaning if the file is not in ram
					if !(this.pool[index].length){return 3;}
					DisksKing * dking; get_disksking_object(dking);
					dking->write(this.pool[index].disk,this.pool[index].diskpos+offset,buf,length);
				}
				else{ memcpy(this.pool[index].contents+offset,amount);}
			return 0;}
		}
	}
	ulong_t close(ulong_t index);				//TODO removes the descriptor from the process' pool
	void evictions_cycle(void);				//removes the contents of files with no listeners and deletes non-disk-backed files from memory

	/*
	Because the more complicated this gets the worse i am simply going to count the processes
	holding a descriptor for the file, it gets called by the syscalls wrappers
	*/
	void writeback_cycle(Virtual_fs * vfs,){
		for (ustd_t i = 0; i < this.length; ++i){
			if (this.pool[i].meta.namelen){
				if !(this.pool[i].listeners){
					if (this.pool[i].pending_sync){ disksync(i);}
					this.pool[i].shared_contents = 0;
				}
			}
		}
	}
	void readmiss(ustd_t findex, Kingmem * mm, ustd_t pages_number, ustd_t pagetype){
		if !(this.descriptions[findex].shared_contents){
			this.descriptions[findex].shared_contents = get_free_identity(mm,pages_number,pagetype);
			disk_read(this.descriptions[findex].disk,this.descriptions[findex].diskpos,this.descriptions[findex].shared_contents,pages_number,pagetype);
			this.descriptions[findex].length = pages_number;        //reminder of the difference between this and meta.length
	        }
        }
};


Class meta{
	uchar_t * name;		//null-ending string btw
	ulong_t namelen;
	ustd_t type;
	ustd_t mode;
	ulong_t length;		//length on disk
};

Class File{
	meta data;
	utsd_t parent_index;

	union{
		//...
		struct{
			ustd_t disk;
			ondisk_t diskpos;
			union{
				//type==DIRECTORY
				struct{
					ustd_t * children;		//indexes into the pool pool
					ustd_t numberof_children;
				};
				//..
				struct{
					//type==STORAGE
					struct{
						ustd_t length;		//in pages, in memory
						ulong_t listeners;	//processes holding a descriptor
						void * shared_contents;	//otherwise SYS_READ reads from shared mappings
						ustd_t pending_sync;
					};
					//type==SWAPFILE
					struct{
						Class Kshm swap;
					};
				};
			};
		};

		//type==DEVICE
		struct{
			uchar_t bus;
			uchar_t device;
			uchar_t irline;
			uchar_t irpin;
			ustd_t geninfo;			//class code...
			ushort_t identification;	//oem low, devid high, because the process ends up being the same anyway
			uchar_t ranges_mask;		//IO space / memory bit
			uchar_t mutlifunction_boo;
			ulong_t bases[6];		//64bit prefetchables in pcibridge...
			ustd_t lengths[6];		//they are not going to need more than that.
			ustd_t expansion_rom;		//holy shit its mikerkode
			struct driver{
				ushort_t length;
				(void)(*) functions;
			};
		};

	};
};
Class Directory : File;
Class Storage : File;
Class Device : File;
Class KingSwap : File;

Class Descriptor{
	ulong_t desc_index;
	ustd_t polled;
	ustd_t flags;
};
