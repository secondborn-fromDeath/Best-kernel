/*
What this guy needs to do:
		pipe arguments into the command port (CF8) and the data port (CFC) and return the read/written values
		device enumeration	-> creating /dev structures, interfacing with acpi for PM function pointers
		the IO apic the interrupt pin numbers (line relative, hence acpi is needed)
*/

enum pcitypes{ PCI_TO_PCI_DEVICE,PCI_TO_PCI_BRIDGE,};
class PciKing{
	King names{
		struct{
		ustd_t identification;
		ustd_t count;
		} * pool : King.pool;
	};

	void pci_init(void){
		this->type = hashtypes::DOUBLE_POINTER;

		Processor * processor = get_processor_object(NUH);
		Device host;
		memset(host,0,sizeof(File));				//host bus is 0,0,0

		boot_write(processor,host,0x71,0x0);					//setting the cpu snoop to be 0 becuase it is fine, see 440fx pciset page 34: "it is whatever"
	}
	/*
	Runtime interface for device drivers
	*/
	void write(Device * dev, ustd_t * source, ulong_t * offsets, ulong_t count){
		Processor * processor = get_processor_object(NUH);
		this->stream_init(NUH);

		ustd_t proto_mask = 1<<31 | dev->bus<<16 | dev->device<<11 | dev->function<<8;
		for (ustd_t g = 0; g < count; ++g){
			ustd_t mask = proto_mask | offsets[g]>>2<<2;	//bit 1 is 32/64bit so we treat it as reserved since i dont support it
			outd(mask,0xcf8);
			outd(source[g],0xcfc);
		}
		__non_temporal this->calendar = 0;
	}
	void read(Device * dev, ustd_t * destination, ulong_t * offsets, ulong_t count){
		Processor * processor = get_processor_object(NUH);
		this->stream_init(NUH);

		ustd_t proto_mask = 1<<31 | dev->bus<<16 | dev->device<<11 | dev->function<<8;
		for (ustd_t g = 0; g < count; ++g){
			ustd_t mask = proto_mask | offsets[g]>>2<<2;
			outd(mask,0xcf8);
			destination[g] = ind(0xcfc);
		}
		__non_temporal this->calendar = 0;
	}

	/*
	Above but for the pci driver itself
	*/
	void boot_write(Processor * processor, Device * dev, ustd_t reg, ustd_t contents){
		ustd_t mask = 1<<31 | dev->bus<<16 | dev->device<<11 | dev->function<<8 | reg;
		outd(mask,0xcf8);
		outd(contents,0xcfc);
	}
	ustd_t boot_read(Processor * processor, Device * dev, ustd_t reg){
		ustd_t mask = 1<<31 | dev->bus<<16 | dev->device<<11 | dev->function<<8 | reg;
		outd(mask,0xcf8);
		return ind(0xcfc);
	}
	/*
	The boot interface
				Create virtual filesystem structures, bind them to a driver and then recurse if the device is a pci bridge

	bus,dev,fun need to be passed as 0 (*==0 lol) initially
	*/
	using enum device_driver_routines;
	void enumerate_devices(Processor * processor, Virtual_fs * vfs, Directory * devices, DriversGod * drivgod, ustd_t bus_number, ustd_t * device_number, ustd_t function, ustd_t re){
		Thread * ori_thread = get_thread_object(NUH);
		for (1; device_number* != 32; ++device_number*){
			Device * newdev = vfs->pool_alloc(1);
			devices->children->pool_alloc(1)* = newdev;	//legal sepple? DANGER
			newdev->bus = bus_number;
			newdev->device = device_number*;
			newdev->function = function;
			function = 0;
			newdev->devsel_timing = get_devsel_timing(processor,newdev);
			newdev->identification = get_idinfo(processor,newdev);
			if (newdev->identification == -1){ vfs->pool_free(newdev,1); continue;}							//first one that can fail AND be checkable

			newdev->geninfo = get_geninfo(processor,newdev);
			ushort_t * irline_pointer = &newdev->irline;
			irline_pointer* = get_irline_and_pin(processor,bus_number,device_number*);
			newdev->function = function;
			ustd_t multifunction_boo;
			ustd_t headertype = get_headertype(processor,newdev,&multifunction_boo);
			set_device_ranges(processor,newdev,headertype);
			newdev->expansion_rom = get_expansionrom(processor,newdev);
			newdev->multifunction_boo = get_multifunction_boolean(processor,newdev);

			newdev -> driver = NULL;
			for (ustd_t g = 0; g < drivgod->length; ++g){
				fiDriv * fidriv = drivgod->pool[g];

				if (fidriv->d->runtime->identification == driver_kinds::MODULE){ continue;}
				if (fidriv->d->runtime->code->classcode != newdev->geninfo){ continue;}
				File * back = malloc(1,pag::SMALLPAGE);
				memset(back,0,4096);
				memcpy(back,newdev,sizeof(File));
				ustd_t irhandler = module_ctl(fidriv,INITIALIZE_DEVICE,newdev);
				memcpy(newdev,back,sizeof(File));
				free(back,1);
				newdev->virt_irhandler = fidriv->d->runtime->userspace->code + fidriv->d->runtime->code->functions[irhandler];

				ustd_t * newdev_name = &newdev->meta.name;

				if (newdev_name* == DISK){
					Disksking * dking = get_disksking_object(NUH);
					dking->load_partition_table(newdev);
				}

				//doing the device's name counter
				ustd_t gj = 0;
				for (0; gj < this->names->length; ++gj){
					if (newdev_name* == this->names->pool[gj]->identification){
						++this->names->pool[gj]->count;
						newdev_name[1] = this->names->pool[gj]->count + 0x30;			//ASCII conversion
						goto pcienum_whatever_goto;
					}
				}

				if (newdev_name* == KEYB){ goto yes_input_dev;
				if (newdev_name* == MOUS){ goto yes_input_dev;
//				if (newdev_name* == TRCK){ goto yes_input_dev;
				goto no_input_dev;
		yes_input_dev:
				File * stream = vfs->pool_alloc(1);
				stream->mem->pool = kptralloc(1);
				stream->mem->pool[0] = malloc(1,pag::SMALLPAGE);
				stream->mem->listeners[0] = 1;
				newdev->double_link = stream;
				stream->parent = devices;
				((ushort_t*)&stream->meta.name)[0] = 'I'|'N'<<8;
				strcpy(&stream->meta.name[2],newdev_name);
		no_input_dev:

				if (newdev_name* == VIDE){
					File * framebuffer = vfs->ramcontents_to_description(newdev->bars[0],newdev->lengths[0],pag::BIGPAGE);
					framebuffer->parent = devices;
					uint32_t * fbname = ((uint32_t*)&framebuffer->meta.name);
					fbname[0] = 0x464D4246;	//FMBF
					fbname[1] = this->names->pool[gj]->count;
				};


				identification_queue * lol = this->names->pool_alloc(1);
				lol->identification = newdev_name*;
				lol->count = 0;									//redundant...
pcienum_whatever_goto:
				Thread * sub = fidriv->runtime->children[0];
				newdev->thread = clone_thread(sub);

				newdev->thread->type = thread_types::DEVICE;
				break;
			}

			if (headertype == PCI_TO_PCI_DEVICE){
				device_enum(processor,vfs,get_secondarybus_number(processor,newdev),0,0,re);	//enumerating the new bus
			}
			if ((multifunction_boo) && (!function)){
				for (ustd_t g = 1; g < 4; ++g){
					device_enum(processor,vfs,drivgod,bus_number,device_number,g,re);
				}
			}
devenum_bail:
		}
	}

	void load_all_devices(void){
		Processor * processor = get_processor_object(NUH);
		Virtual_fs * vfs = get_vfs_object(NUH);
		Directory * devices = &vfs->descriptions[1];
		Driversgod * drivgod = get_driversgod_object(NUH);

		this->names->pool = malloc(2,pag::SMALLPAGE);
		this->names->length = 1028;						//nuff

		for (ustd_t i = 0; i < devices->children->length; ++i){			//marking devices
			devices->children->pool[i]->present = 0;}
		ustd_t device_number = 0;
		enumerate_deivces(processor,vfs,devices,drivgod,0,0,&device_number,0);	//filling them
		for (ustd_t i = 0; i < devices->children->length; ++i){			//marking devices
			Device * device = &devices->children->pool[i];
			if (device->present == 0){					//removing hotplugged-out
				ustd_t * devname = &device->meta.name;
				if (device->devname == DISK){				//if the device was a disk removing the partition table...
					Disksking * dking = get_disksking_object(NUH);
					for (ustd_t o = 0; o < dking->partitions->length; ++o){
						Partition * part = &dking->partitions->pool[o];
						if (part->disk == device){
							dking->partitions->pool_free(part,1);
					}
				}
				DEVKILL(&devices->children->pool[i];}
		}

		kptr->pool_free(this->names->pool,2);
	}
	/*
	A bunch of get/set_XXX info from configuration space -type functions
		i dont do cardbus, expresscard uses the type 0x0 header
	*/
	ustd_t get_devsel_timing(Processor * processor, Device * dev){
		ustd_t mask = 1<<31 | dev->bus<<16 | dev->device<<11 | 0x4;
		outd(mask,0xcf8);
		mask = ind(0xcfc);
		return mask<<22>>30;
	}
	ustd_t get_headertype(Processor * processor, Device * dev, ustd_t * multifunction){
		ustd_t ret =  boot_read(processor,dev,0x6) <<16>>24;
		if (ret & 128){ multifunction* = 1; ret &= 127;}		//signing the boolean and masking the bit
		return ret;
	}
	ushort_t get_irline_and_pin(Processor * processor, Device * dev){	//returns them packed into a WORD, line is lowbyte, pin is high
		return boot_read(processor,dev,0x3c) <<16>>16;
	}
	ustd_t get_geninfo(Processor * processor, Device * dev){
		return boot_read(processor,dev,0x8);
	}
	ustd_t get_idinfo(Processor * processor, Device * dev){
		return boot_read(processor,dev,0x0);
	}
	ustd_t get_command(Processor * processor, Device * dev){
		return boot_read(processor,dev,0x4)<<16>>16;
	}
	ustd_t set_command(Processor * processor, Device * dev, ustd_t contents){
		return boot_write(processor,dev,0x4,contents)<<16>>16;
	}
	ustd_t get_status(PRocessor * processor, Device * dev){
		return boot_read(processor,dev,0x4)>>16;
	}
	ustd_t get_expansionrom(Processor * processor, Device * dev, ustd_t headertype){
		ustd_t reg = 0x30;
		if (headertype == PCI_TO_PCI_DEVICE){ reg += 8;}
		return boot_read(processor,dev,reg);
	}
	ustd_t get_secondarybus_number(Processor * processor, Device * dev){
		return boot_read(processor,dev,0x18);
	}
	/*
	This is a bit dense but if you ignore the looping due to teh bigger BARs it is easy, also we are returning to skip already-used bars in the caller
	*/
	ustd_t get_bar_info(Processor * processor, Device * dev, ustd_t rangenum, ustd_t reg){	//i got the procedure from osdev.org/PCI#Address_and_size_of_the_BAR
		ustd_t retu = 0;
		ustd_t or = 0;		//gets increased by 32 on 64bit bar
		ustd_t loop = 1;

			//i am thinking that the loop should be up here and i would not be increasing it and instead breaking? increase it but only once probably.

		for (1; loop; --loop){
			//backing up the base of the range
			ustd_t base = boot_reg(processor,dev,reg);
			if !(base & 1){					//testing the first bit for serial ports space vs memory
				if (base & 3){				//testing the type field for 64bit BAR
					if !(or){ ++loop;
					retu += 2;}			//no such thing as a 96 bit BAR...
				}
				if (base & 4){				//testing wether theaddress range is prefetcheable
					dev->types[rangenum] = 2;
				}else{ dev->types[rangenum] = 0;}
			}
			else{
				dev->types[rangenum] = 1;
			}
			dev->bases[rangenum] = NULLPTR;
			dev->lengths[rangenum] = NULLPTR;
			dev->bases[rangenum] |= (base>>4<<4)<<or;
			ustd_t dis = get_command(processor,dev);					//taking away the device's ability to master south and northbridge
			set_command(processor,dev,dis<<2>>2);
			boot_write(processor,dev,reg,-1);						//reading the size of the range by writing all 1s to the BAR register and then reading
			dev->lengths[rangenum] |= ((!(boot_read(processor,dev,reg)<<2>>2))+1))<<or;	//removing type bits and computing two's complement

			//restoring everything
			boot_write(processor,dev,reg,base);
			set_command(processor,dev,dis);

			or += 32;
		}
	}
	/*
	Things that are not bars are SUPER silly in pcibridge so they get done by hand
	as long as the resulting structures are fine...
	*/
	void set_device_ranges(Processor * processor, Device * dev, ustd_t headtype){
		memset(dev->bases,0,sizeof(dev->bases*)*6);
		memset(dev->lengths,sizeof(dev->lengths*)*6);
		memset(dev->types,0,sizeof(dev->types*)*6);

		if (headtype == PCI_TO_PCI_DEVICE){
			ustd_t regoff = 0x10;
			for (ustd_t i = 0; i < 6; ++i){ i += get_bar_info(processor,dev,i,regoff);}
		}
		else{	//PCI_TO_PCI_BRIDGE
			get_bar_info(processor,dev,0,0x10);
			get_bar_info(processor,dev,0,0x14);
			//memory range field
			ustd_t overarching_dword = boot_read(processor,dev,0x20);
			dev->bases[2] = overarching_dword<<16>>16;
			dev->lengths[2] = overarching_dword>>16 - dev->bases[2];
			dev->types[2] = 0;
			//serial ports field
			overarching_dword = boot_read(processor,dev,0x1c);
			ustd_t helper_dword = boot_read(processor,dev,0x30);
			dev->bases[3] = overarching_dword<<24>>24 | helper_dword<<16>>16;
			dev-lengths[3] = (overarching_dword<<16>>24 | helper_dword>>16) - dev->bases[3];
			dev->types[3] = 1;
			//prefetchable field
			overarching_dword = boot_read(processor,dev,0x24);
			dev->bases[4] = overarching_dword<<16>>16 | boot_read(processor,dev,0x28);
			dev->lengths[4] = overarching_dword>>16 | boot_read(processor,dev,0x2c) - dev->bases[4];
			dev->types[3] = 2;
		}
	}

};
