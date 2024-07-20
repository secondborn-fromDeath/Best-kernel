//booting directly into the kernel because i dont like for the majority of the code to be duplicated

/*
 This is the function that starts the kernel
 /*
  * Workflow (i am considering that drivers are built into the kernel for now):
  *		disable >32 interrupts
  *		enable A20 line								//idc for the 8086 address space read it causes problems in long mode
  *		mode switch up to 64bit
  *		set up memory (MSRs too) and kerneldata
  *		set up acpi
  *		fill above with acpi info
  *		set up the global descriptors table and the interrupt dt		//with the kernel functions
  *		exit bootservices							//no setvirtualmap because i do identity mapping
  *		ensure POST was performed, ifnot turn machine off (acpi)
  *		pci enumeration
  *		get into IOAPIC mode
  *		do interrupt vectors and enable interrupts
  *		wake online-capable processors into mode switching and HLT
  *		load and execute specified initsys (config file)
  *		jump bsp to scheduler
  */

