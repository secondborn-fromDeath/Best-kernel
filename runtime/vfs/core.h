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
		if !(newfren){ return newfren;}
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
			if !(file->shared_contents->pool[page]){ return NULLPTR;}
			dking->read(file->disk,file->diskpos+page*mm->get_multi_from_pagetype(file->shared_contents->mapped_pagetype),file->shared_contents->pool[page],1,file->shared_contents->mapped_pagetype);
		}
		return file->shared_contents->pool[page];
	}
	//cant you just be a normal person and ask for the number of directories and files under the partition in the first place?
	void recursive_insert(DisksKing * dking, Directory * dir){
		if (dir->type != filetypes.DIRECTORY){ return;}
		Kingptr * kptr = get_kingpointer_object(void);
		dir->children->pool = kptr->pool_alloc(dir->children->count);
		dking->getdir_entries(dir);
		for (ustd_t u = 0; u < dir->children->count; ++u){
			File * newfag = this->alloc(1);					//doing "weak" allocations so there is no mutex, see kontrol.h
			File * newfag = dking->load_file(dir->children->pool[u],newfag);
			dir->children->pool[u] = newfag;				//substituting for the vfs pointer
			newfag->parent = dir;
			recursive_insert(dir->children->pool[u],newfag);
		}
	}
	void mount(ustd_t part){
		DisksGod * dgod = get_disksgod_object(void);
		Virtual_fs * vfs = get_vfs_object(void);
		vfs->stream_init(void);

		if (vfs->length-vfs->count < dgod->partitions->pool[part]->files_number){
			__non_temporal vfs->calendar = 0;
			return 1;
		}

		File * root = dgod->load_file(part,root);
		recursive_insert(dgod,root);
		__non_temporal vfs->calendar = 0;
		return 0;
	}
	/*
	This does filename recursion and returns the index into the filename (directory) at which it failed or something
	on success returns 0, returns a pointer to however far was reached into the string, index returns the index into the file*/
	enum recurse_directories_errors{ NOTA_DIR=1,NO_MATCHING_FILENAME,};
	ulong_t recurse_directories(uchar_t * path, Directory * dir, char ** out, ustd_t * index){
		for (ustd_t i = 0; i < dir->chidlren->length; ++i){
			if !(dir->ckarray[i]){ continue;}
			ustd_t bigcount = 0;
			File * test = dir->children->pool[i];
			for (ustd_t o = 0; path[o] == test->meta.name[o]; ++o){
				if !(path[o]){
					if (path[o] != test->meta.name[o]){ break;}
					out* = &path[o];
					index* = (test-this->descriptions)/sizeof(File);
					return 0;
				}else{
					if (path[o] == '/'){
						if (test->type != file_types::DIRECTORY){ return 1;}
						recurse_directories(path[o+1],test,out);
					}
				}
			}
		}
		return 2;
	}
	/*
	This returns the index into however deep we managed to go into the path (and tells you about that too...)*/
	enum open_flags{ O_CREAT,};
	ulong_t open(uchar_t * path, ulong_t dir_index, ustd_t * strlen_ret, ustd_t * index_ret){	//can do mkfile from here, compressing because it is one check anyway and we do disksync
		if !(this->ckarray[dir_index]){ return 1;}
		char * lil;
		ustd_t p = recurse_directories(path,&this->pool[dir_index],&lil,index_ret);
		strlen_ret* = lil-path;
		return p;
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
class Swapfile : memfile{	//partitions
	ustd_t disk;
	ustd_t partition;
};

class meta{
	uchar_t name[64]		//null-ending string, if you need more, you dont
	ustd_t type;
	ustd_t mode;			//syntax is 1 == root only, 0 == all, from MSB to LSB:		0,0,0,0,0,0,writing,reading,
};

class File{
	meta data;
	File * parent;
	File ** double_link;				//to do parent->children->pool_free()

	union{
	//type==DEVICE
	struct{
		ulong_t kind;				//string indicating the type of device, assigned by the drivers and is meant for use by the subsystem modules
		union{
			ulong_t name;			//something conjured up by the kernel so that IO operations dont rely on breakable indexes
			ustd_t identification;		//oem low, devid high, because the process ends up being the same anyway
		};
		ustd_t geninfo;				//class code...
		uchar_t bus;
		uchar_t device;
		uchar_t irline;
		uchar_t irpin;
		uchar_t ranges_mask;			//IO space / memory bit
		uchar_t multifunction_boo;
		ulong_t bases[6];			//64bit prefetchables in pcibridge...
		ulong_t lengths[6];			//they are not going to need more than that.
		char types[6];				//0 is uncacheable memory, 1 is serial ports and 2 is prefetcheable memory
		ustd_t expansion_rom;			//holy shit its mikerkode
		char * ACPI_definition;			//honestly, acpi_definition needs to be a big class because of how things actually work (poll_status... pwr... whatever you want), this is a placeholder
		Thread * thread;
		Driver * driver;
		void * virt_irhandler;
	};
	struct{
		union{
		//...
		struct{
			ustd_t partition;		//filesystem type...
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
							ustd_t listeners;		//what stops the unmounting
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
