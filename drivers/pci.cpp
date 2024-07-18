/*
What this guy needs to do:
		pipe arguments into the command port (CF8) and the data port (CFC) and return the read/written values
		device enumeration	-> creating /dev structures, interfacing with acpi for PM function pointers
		the IO apic the interrupt pin numbers (line relative, hence acpi is needed)
*/

enum pcitypes{ PCI_TO_PCI_DEVICE,PCI_TO_PCI_BRIDGE,};
Class PciKing : King{
	/*
	Runtime interface for device drivers
	*/
	void write(usdt_t dev, ustd_t function, ustd_t * source, ulong_t * offsets, ulong_t count){
		Virtual_fs * vfs; get_vfs_object(vfs);
		Processor * processor = get_processor_object();
		ustd_t proto_mask = 1<<31 | vfs->descriptions[dev]->bus<<16 | vfs->descriptions[dev]->device<<11 | function<<8;
		for (ustd_t g = 0; g < count; ++g){
			ustd_t mask = proto_mask | offsets[g]>>2<<2;	//bit 1 is 32/64bit so we treat it as reserved since i dont support it
			__asm__(
			"OUT %%eax,$0xcf8\n\t"
			"movl %1,%%eax\n\t"
			"OUT %%eax,$0xcfc\n\t"
			::"r"(mask),"m"(source[g]):
			);
			__attribute__((optimize("O0"))) for (ustd_t h = processor->frequency/this.frequency*dev->devsel_timing; h; --h);	//either 3 or 4
		}
	}
	void read(ustd_t dev, ustd_t * destination, ulong_t * offsets, ulong_t count){
		Virtual_fs * vfs; get_vfs_object(vfs);
		Processor * processor = get_processor_object();
		ustd_t proto_mask = 1<<31 | vfs->descriptions[dev]->bus<<16 | vfs->descriptions[dev]->device<<11 | function<<8;
		for (ustd_t g = 0; g < count; ++g){
			ustd_t mask = proto_mask | offsets[g]>>2<<2;
			__asm__(
			"OUT %%eax,$0xcf8\n\t"
			"IN $0xcfc,%%eax\n\t"
			"movl %%eax,%1\n\t"
			::"r"(mask),"m"(destination[g]):
			);
			//here bus frequency is either 33Mhz or 66Mhz, devsel_timing is either 3, 4 or assumed 16
			__attribute__((optimize("O0"))) for (ustd_t h = processor->frequency/this.frequency*dev->devsel_timing; h; --h);
		}
	}

	/*
	Above but for the pci driver itself
	*/
	void boot_write(Processor * processor, Device * dev, ustd_t reg, ustd_t contents){
		__attribute__((optimize("O0"))){ for (ustd_t h = processor->frequency/this.frequency*dev->devsel_timing; h; --h);
		}
		ustd_t mask = 1<<31 | vfs->descriptions[dev]->bus<<16 | vfs->descriptions[dev]->device<<11 | reg;
		__asm__(
		"OUT %%eax,$0xcf8\n\t"
		"mov %%ecx,%%eax\n\t"
		"IN %%eax,$0cfc\n\t"
		::"r"(mask),"r"(contents):
		);
	}
	ustd_t boot_read(Processor * processor, Device * dev, ustd_t reg){
		__attribute__((optimize("O0"))){ for (ustd_t h = processor->frequency/this.frequency*dev->devsel_timing; h; --h);
		}
		ustd_t mask = 1<<31 | vfs->descriptions[dev]->bus<<16 | vfs->descriptions[dev]->device<<11 | reg;
		__asm__(
		"OUT %%eax,$0xcf8\n\t"
		"IN $0xcfc,%%eax\n\t"
		::"r"(mask):
		);
		return mask;
	}
	/*
	The boot interface
				Create virtual filesystem structures, bind them to a driver and then recurse if the device is a pci bridge

	bus,dev,fun need to be passed as 0 (*==0 lol) initially
	*/
	ustd_t device_enum(Processor * processor, Virtual_fs * vfs, DriversGod * drivgod, ustd_t bus_number, ustd_t * device_number, ustd_t function){
		if (device_number* == 32){ return 0;}	//out
		Device * newdev = vfs->pool_alloc(1);
		newdev->bus = bus_number;
		newdev->device = device_number*;
		newdev->function = function;
		newdev->devsel_timing = get_devsel_timing(processor,newdev);	//see boot_read/write
		newdev->identification = get_idinfo(processor,newdev);
		if !(newdev->identification == -1){				//first one that can fail AND be checkable
			++this.enumerated_devices;
			newdev->geninfo = get_geninfo(processor,newdev);
			((ushort_t*)&newdev->irline)[0] = get_irline_and_pin(processor,bus_number,device_number*);
			ustd_t multifunction_boo;
			ustd_t headertype = get_headertype(processor,newdev,&multifunction_boo);
			set_device_ranges(processor,newdev,headertype);
			newdev->expansion_rom = get_expansionrom(processor,newdev);
			newdev->multifunction_boo = get_multifunction_boolean(processor,newdev);

			newdev -> driver = NULL;	//NOTE ioctl needs to check for this
			for (ustd_t g = 0; g < drivgod->length; ++g){
				if !(vfs->descriptions[drivgod->drivers[g]]->driv->code->check_classcode(newdev->geninfo)){	//ugly but its too long
				if !(vfs->descriptions[drivgod->drivers[g]]->driv->code->attach_model(newdev))){
					break;}}
			}
			if (headertype == PCI_TO_PCI_DEVICE){
				device_enum(processor,vfs,get_secondarybus_number(processor,newdev));
			}
			if ((multifunction_boo) && (!function)){
				for (ustd_t g = 1; g < 4; ++g){
					device_enum(processor,vfs,drivgod,bus_number,device_number,g);
				}
			}

		}
		else{vfs->pool_free(newdev,1);}
		++device_number*;
		return 1;	//keep going
	}
		/*
	A bunch of get/set_XXX info from configuration space -type functions
		i dont do cardbus, expresscard uses the type 0x0 header
	*/
	ustd_t get_devsel_timing(Processor * processor, Device * dev){
		ustd_t mask = 1<<31 | dev->bus<<16 | dev->device<<11 | 0x4;
		__asm__ (
		"OUT %%eax,$0xcf8\n\t"
		"IN $0xcfc,%%eax\n\t"
		::"r"(mask):"%rcx"
		);
		return mask<<22>>30;	//extracting the 2 bits from status-command
	}
	ustd_t get_headertype(Processor * processor, Device * dev, ustd_t * multifunction){
		ustd_t ret =  boot_read(processor,dev,0x6) <<16>>24;
		if (ret & 128){ multifunction* = 1; ret &= 127;}		//signing the boolean and masking the bit
		return ret;
	}
	ushort_t get_irline_and_pin(Processor * processor, Device * dev){	//returns them packed into a WORD
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
	This function is going to return the base, type and size of the address range
	The other function does the header decoding part and calls on this ONLY for the BARs
																										the File class is getting really ugly...
	*/
	void get_bar_info(Processor * processor, Device * dev, ustd_t rangenum, ustd_t reg){	//i got the procedure from osdev.org/PCI#Address_and_size_of_the_BAR
		//backing up the base of the range
		ustd_t base = boot_reg(processor,dev,reg);
		uchar_t type = base <<31>>31;
		for (ustd_t i = 0; i < rangenum; ++i){ type<<=1;}
		dev->bases[rangenum] = base >>1<<1;
		dev->range_mask |= type;
		//disabling anything memory and serial ports access because of bad behaviour that may be caused otherwise by the next step
		ustd_t dis = get_command(processor,dev);
		set_command(processor,dev,dis<<2>>2);
		//reading the size of the range by writing all 1s to the BAR register and then reading
		boot_write(processor,dev,reg,-1);
		dev->lengths[rangenum] = (~(boot_read(processor,dev,reg)<<2>>2))+1;	//removing type bits and computing two's complement

		//restoring everything
		boot_write(processor,dev,reg,base);
		set_command(processor,dev,dis);
	}
	/*
	Things that are not bars are SUPER silly in pcibridge so they get done by hand
	as long as the resulting structures are fine...
	*/
	void set_device_ranges(Processor * processor, Device * dev, ustd_t headtype){
		if (headtype == PCI_TO_PCI_DEVICE){
			ustd_t regoff = 0x10;
			constexpr for (ustd_t i = 0; i < 6; ++i){ get_bar_info(processor,dev,i,regoff);}
		}
		else{	//PCI_TO_PCI_BRIDGE
			get_bar_info(processor,dev,0,0x10);
			get_bar_info(processor,dev,0,0x14);
			//memory range field
			ustd_t overarching_dword = boot_read(processor,dev,0x20);
			dev->bases[2] = overarching_dword<<16>>16;
			dev->lengths[2] = overarching_dword>>16 - dev->bases[2];
			//serial ports field
			overarching_dword = boot_read(processor,dev,0x1c);
			ustd_t helper_dword = boot_read(processor,dev,0x30);
			dev->bases[3] = overarching_dword<<24>>24 | helper_dword<<16>>16;
			dev-lengths[3] = (overarching_dword<<16>>24 | helper_dword>>16) - dev->bases[3];
			//prefetchable field
			overarching_dword = boot_read(processor,dev,0x24);
			dev->bases[4] = overarching_dword<<16>>16 | boot_read(processor,dev,0x28);
			dev->lengths[4] = overarching_dword>>16 | boot_read(processor,dev,0x2c) - dev->bases[4];
		}
	}

};
