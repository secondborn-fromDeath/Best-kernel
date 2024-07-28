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
	CONNECTION,		//this shoudlnt be here...
};

class Virtual_fs : King{
	void scan_partitions(void);	//returns pointers and ids into an array
	Partlayout_driver partlay[???];
	void insert_file(void);		//filesystem-style translator
	Filesystem_driver fs[??];

	File * pool : King.pool;
	extern get_rand32(void) : King.get_rand(void);



	void template_constructor(File * newfile, File * parent, char * name, ustd_t type){
		File ** term = parent->children->pool_alloc(1);
		term* = newfile;
		strcpy(&newfile->meta.name,name);
		newfile->meta.type = type;
	}
	#define storage_constructor(a,b,c){ template_constructor(a,b,c,filetypes.STORAGE)}
	#define directory_constructor(a,b,c){ template_constructor(a,b,c,filetypes.DIRECTORY); a->children->length = 0; a->children->count = 0; a->children->pool = 0}

	Storage * ramcontents_to_description(void * contents, ulong_t length, ustd_t pagetype){
		Storage * newfren = this.pool_alloc(1);
		ustd_t multi = get_multi_from_pagetype(pagetype);
		ustd_t i;
		for (i = 0; i < lenght; ++i){
			newfren->shared_contents[i] = contents+i*multi;
		}
		newfren->mapped_pagetype = pagetype;
		newfren->meta.length = (newfren->shared_contents[i] - contents)*(multi/4096);
		return newfren;
	}

	//loads a page of a file on disk if it is even needed
	void * load_page(DisksGod * dking, Storage * file, ustd_t page){
		if (page > file->length){ return 1;}
		if !(file->shared_contents[page]){
			file->shared_contents[page] = malloc(1,file->mapped_pagetype);	//NOTE HARDENING
			dking->read(file->disk,file->diskpos+page*get_multi_from_pagetype(file->mapped_pagetype),file->shared_contents[page],1,file->mapped_pagetype);
		}
		return file->shared_contents[page];
	}
	void recursive_insert(FS_Driver * fs, Directory * dir){
		if (dir->type != filetypes.DIRECTORY){ return;}
		Kingptr * kptr = get_kingpointer_object(void);
		dir->children = kptr->pool_alloc(dir->numberof_children);	//hardening...
		fs->getdir_entries(dir);
		for (ustd_t u = 0; u < dir->numberof_children; ++u){
			File * newfag = this.pool_alloc(1);
			fs->load_file(dir->children[u],newfag);
			dir->children[u] = newfag;				//substituting for the vfs pointer
			recursive_insert(dir->children[u],newfag);
		}
	}
	void mount(ustd_t disk, ustd_t part){
		DisksGod * dgod = get_disksgod_object(void);
		FS_Driver * fs = dgod->disks[disk]->pool[part];		//NOTE HARDENING

		File * root = this.pool_alloc(1);
		fs->load_file(fs->meta.diskpos,root);
		recursive_insert(fs,root);
	}
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
		return stand[0];
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
					DisksKing * dking = get_disksking_object(void);
					dking->stream_init(void);
					dking->read(buf,this.pool[index].disk,this.pool[index].diskpos+offset,length);
					__non_temporal dking->calendar = 0;
				}
				else{ memcpy(buf,this.pool[index].contents+offset,amount);}
			return 0;}
		}
	}
	ustd_t write(ulong_t index, void * buf, ulong_t amount, offset){
		if (index == get_kontrol_object(void)->get_framebuffer_findex(void)){
			ps2_enable_device(MOUSE);
			ps2_enable_device(KEYBOARD);
		}
		if (offset > this.pool[index].meta.length){ return 4;}
		switch (this.pool[index].type){
			case DIRECTORY:{ return 1;}
			case DEVICE:{ return 2;}
			default:{
				if (this.pool[index].contents >> 50){	//meaning if the file is not in ram
					if !(this.pool[index].length){return 3;}
					DisksKing * dking; get_disksking_object(dking);
					dking->stream_init(void);
					dking->write(this.pool[index].disk,this.pool[index].diskpos+offset,buf,length);
					__non_temporal dking->calendar = 0;
				}
				else{ memcpy(this.pool[index].contents+offset,amount);}
			return 0;}
		}
	}
	ulong_t close(ulong_t index);				//TODO removes the descriptor from the process' pool

	/*
	See the File object for the way i do listeners to files
	*/
	void writeback_cycle(Virtual_fs * vfs,){
		DisksKing * dking = get_disksking_object(void);

		for (ustd_t i = 0; i < this.length; ++i){
			if (this.ckarray[i]){
				File * file = &this.descriptions[i];
				ustd_t filemulti = get_multi_from_pagetype(file->mapped_pagetype);
				ustd_t closedyet = 0;	//we sync to disk on lazy syncing when no listeners are present
				for (ustd_t h = 0; h < file->pages_count; ++h){
					if !(file->listeners[h]){
						dking->write(file->disk,file->diskpos + filemulti*h,file->shared_contents[h],1,file->mapped_pagetype);
						Kingmem * mm =  get_kingmem_object(void);
						mm->manipulate_phys((file->shared_contents[i])<<5>>18<<1,1,file->mapped_pagetype,actions.CLEAR);
						mm->used_memory -= get_multi_from_pagetype(file->mapped_pagetype)/4096;
					}
					else{ ++closedyet;}
				}
				if ((closedyet == file->pages_count) && (file->pending_sync)){ dking->sync(file);}	//NOTE there is some nuance to that function obviously...
			}
		}
	}
};


class memfile : Kshm{
	struct{ char[4096]} ** pool : Kshm.pool;
};

class meta{
	uchar_t * name;		//null-ending string btw
	ustd_t type;
	ustd_t mode;
	ulong_t length;		//length on disk in smallpages
	ustd_t disk;
	ondisk_t diskpos;
};

class File{
	meta data;
	File * parent;

	union{
	//type==DEVICE
	struct{
		uchar_t bus;
		uchar_t device;
		uchar_t irline;
		uchar_t irpin;
		ustd_t geninfo;			//class code...
		ushort_t identification;	//oem low, devid high, because the process ends up being the same anyway
		uchar_t ranges_mask;		//IO space / memory bit
		uchar_t multifunction_boo;
		ulong_t bases[6];		//64bit prefetchables in pcibridge...
		ulong_t lengths[6];		//they are not going to need more than that.
		char types[6];			//0 is uncacheable memory, 1 is serial ports and 2 is prefetcheable memory
		ustd_t expansion_rom;		//holy shit its mikerkode
		Driver * driver;
		ustd_t maxfunc;			//max function number under ioctl
		King mut;
	};
	struct{
		union{
		//...
		struct{
			union{
				//type==DIRECTORY
				struct{
					Kptr children;
				};
				//..
				struct{
					void ** shared_contents;
					ustd_t mapped_pagetype;
					union{
						//type==STORAGE
						struct{
							ustd_t mapped_pagetype;
							ustd_t length;		//used with mapped_pagetype
							uchar_t * listeners;	//listeners are per-page and not per-file
							void ** shared_contents;
							ustd_t pending_sync;
						};

						//type==SWAPFILE
						memfile swap;
					};
				};
			};
		};
	};
	//type==DRIVER
	Driver d;
	};
};
class Directory : File;
class Storage : File;
class Device : File;
class KingSwap : File;
class fiDriv : File;

class Descriptor{
	ulong_t desc_index;
	ustd_t polled;
	ustd_t flags;		//the first bit of flags indicates a file mapping if set
};
