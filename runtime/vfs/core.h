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
		parent->children->pool_alloc(1) = newfile;
		strcpy(&newfile->meta.name,name);
		newfile->meta.type = type;
	}
	#define storage_constructor(a,b,c){ template_constructor(a,b,c,filetypes.STORAGE)}
	#define directory_constructor(a,b,c){ template_constructor(a,b,c,filetypes.DIRECTORY); a->children->length = 0; a->children->count = 0; a->children->pool = 0}

	//NOTE unindexable description
	Storage * ramcontents_to_description(void * contents, ulong_t length, ustd_t pagetype){
		Kingmem * mm = get_kingmem_object(void);
		Storage * newfren = this->descriptions_alloc(1);
		ustd_t multi = mm->get_multi_from_pagetype(pagetype);
		ustd_t i;
		for (i = 0; i < lenght; ++i){
			newfren->shared_contents->pool[i] = contents+i*multi;
		}
		newfren->shared_contents->mapped_pagetype = pagetype;
		newfren->meta.length = (newfren->shared_contents->pool[i] - contents)*(multi/4096);
		return newfren;
	}

	//loads a page of a file on disk if it is even needed
	void * load_page(DisksGod * dking, Storage * file, ustd_t page){
		Kingmem * mm = get_kingmem_object(void);
		if (page > file->shared_contents->length){ return 1;}
		if !(file->shared_contents->pool[page]){
			file->shared_contents->pool[page] = malloc(1,file->shared_contents->mapped_pagetype);	//NOTE HARDENING
			dking->read(file->disk,file->diskpos+page*mm->get_multi_from_pagetype(file->shared_contents->mapped_pagetype),file->shared_contents->pool[page],1,file->shared_contents->mapped_pagetype);
		}
		return file->shared_contents->pool[page];
	}
	//i dont really know what is going on here
	void recursive_insert(FS_Driver * fs, Directory * dir){
		if (dir->type != filetypes.DIRECTORY){ return;}
		Kingptr * kptr = get_kingpointer_object(void);
		dir->children->pool = kptr->pool_alloc(dir->children->count);	//hardening...
		fs->getdir_entries(dir);
		for (ustd_t u = 0; u < dir->children->count; ++u){
			File * newfag = this->descriptions_alloc(1);
			fs->load_file(dir->children->pool[u],newfag);
			dir->children->pool[u] = newfag;				//substituting for the vfs pointer
			recursive_insert(dir->children->pool[u],newfag);
		}
	}
	void mount(ustd_t disk, ustd_t part){
		DisksGod * dgod = get_disksgod_object(void);
		FS_Driver * fs = dgod->disks[disk]->pool[part];		//NOTE HARDENING

		File * root = this->descriptions_alloc(1);
		fs->load_file(fs->meta.diskpos,root);
		recursive_insert(fs,root);
	}
	/*
	All of these take indexes to directories and relative paths and return indexes to Files, userspace wrappers use the local Descriptor pool
	*/
	ulong_t recurse_directories(uchar_t * path, ulong_t dir, ustd_t * stand){
		for (ustd_t g = 0; g < this->descriptions[dir]->active_cnt; ++g){
			ulong_t attempt = this->descriptions[this->descriptions[dir]->children->pool[g]];
			for (ustd_t i = 0; 1; ++i){
				if ((path[i] == '/') && (this->descriptions[attempt]->meta.name[i] == 0)){
					stand[0] = attempt;	//so that i can return the last directory that was successful
					stand[1] = i+1;
					ulong_t t = recurse_directories(path[i+1],attempt);
					if (t != -1){ return t;}
					break;	//just so something silly in fs wont affect here
				}
				if (path[i] != this->descriptions[attempt]->meta.name[i]){ break;}
				if !(path[i]){ return attempt;}	//success
			}
		}
		return -1;	//fail D:
	}

	ulong_t open(uchar_t * path, ulong_t dir_index, ustd_t flags){	//can do mkfile from here, compressing because it is one check anyway and we do disksync
		void * desc_ptr = this->descriptions;
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
		Storage * file = &vfs->descriptions[index];
		if (offset > file->meta.length){ return 4;}
		switch (file->type){
			case DIRECTORY:{ return 1;}
			case DEVICE:{ return 2;}
			default:{
				DisksKing * dking = get_disksking_object(void);
				dking->read(file->disk,file->diskpos+offset,buf,amount);
			return 0;}
		}
	}
	ustd_t write(ulong_t index, void * buf, ulong_t amount, offset){
		if (index == get_kontrol_object(void)->framebuffer_index){
			ps2_enable_device(MOUSE);
			ps2_enable_device(KEYBOARD);
		}
		Storage * file = &this->descriptions[index];
		if (offset > file->meta.length){ return 4;}
		switch (file->type){
			case DIRECTORY:{ return 1;}
			case DEVICE:{ return 2;}		//NOTE reminder to not make the framebuffer a device structure
			default:{
				DisksKing * dking = get_disksking_object(void);
				dking->write(file->disk,file->diskpos+offset,buf,amount);	//reminder disk 0 is MEMORY
			return 0;}
		}
	}
	ulong_t close(ulong_t index);				//TODO removes the descriptor from the process' pool	reminder to not do silly shrinkage shit

	/*
	See the File object for the way i do listeners to files
	*/
	void writeback_cycle(Virtual_fs * vfs,){
		DisksKing * dking = get_disksking_object(void);
		for (ustd_t i = 0; i < this->length; ++i){
			if (this->ckarray[i]){
				Kingmem * mm = get_kingmem_pbject(void);
				File * file = &this->descriptions[i];
				ustd_t filemulti = mm->get_multi_from_pagetype(file->shared_contents->mapped_pagetype);
				ustd_t closedyet = 0;	//we sync to disk when no listeners are present and a pending sync is on
				for (ustd_t h = 0; h < file->pages_count; ++h){
					if !(file->listeners[h]){
						dking->write(file->disk,file->diskpos+filemulti*h,file->shared_contents->pool[h],filemulti);
						Kingmem * mm =  get_kingmem_object(void);
						mm->manipulate_phys((file->shared_contents->pool[i])<<5>>18<<1,1,file->shared_contents->mapped_pagetype,actions.CLEAR);
						mm->used_memory -= get_multi_from_pagetype(file->shared_contents->mapped_pagetype)/4096;
					}
					else{ ++closedyet;}
				}
				if ((closedyet == file->pages_count) && (file->pending_sync)){ dking->sync(file);}	//NOTE there is some nuance to that function obviously...
			}
		}
	}
};


class memfile : Hash{
	char * listeners : Hash.ckarray;
	ustd_t mapped_pagetype;
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
		char * ACPI_definition;		//honestly, acpi_definition needs to be a big class because of how things actually work (poll_status... pwr... whatever you want), this is a placeholder
		Thread * thread;
		void * virt_irhandler;
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
					union{
						//type==STORAGE
						struct{
							memfile mem;
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

	//type==QUIRK
	drivQuirk d;
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
