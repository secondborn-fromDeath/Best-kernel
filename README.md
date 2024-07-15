# Best-kernel
better than even loonix.

# "why is it better"
because drivers on linux run on ring0 with the vm_to_phys() function which is a massive security threat because it takes nothing for my driver to set up its own paging (with knowledge of where the instruction pointer and stack are to avoid segfaults), set up its own gdt and overwrite the firmware.
In my operating systems drivers "run" on userland and use system calls to access devices.

# Updates
may take anywhere between a day and weeks depending on what it is I have implemented

Some of the code might seem like nonsense and if that is the case I can assure you that I have a note for it, ergo: "why are mutexes not being used anywhere???"

# Contributing
the kernel is a solo project but device drivers (maybe even userspace applications???) are welcome

# License
Definitions:

'Software' is any document distributed along this License

'Plausibly alterated' means any document that may be be used to re-generate the Software by employing the reverse "logic" (standing for both typewriter and computer) that was used to generate the document in question, meaning that all direct compilation, transpilation->compilation, or compilation into bytecode to be run through an emulator are forbidden by this License (see the Statement).


Statement:

You must not distribute the Software, be this through an "original work" or straight up plagiarism.

You may modify and run the Software for your personal, individual use and you may distribute (possibly for a fee) documentation for the Software.
