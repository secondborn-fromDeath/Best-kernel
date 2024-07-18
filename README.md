# Best-kernel
better than even loonix.

# "why is it better"
Linux drivers run on ring zero, which is as much privilege as the kernel has, with some tinkering things like "patching" the firmware
are doable from a driver, idc that it is open source nobody has ever reviewed the 90 megabytes that is ACPICA.
In my operating system (that will come too) drivers "run" on userland and use system calls to access devices (bit of a mess...).

# Updates
may take anywhere between a day and weeks depending on what it is I have implemented

Some of the code might seem like nonsense and if that is the case I can assure you that I have a note for it, ergo: "why are mutexes not being used anywhere???"

# Contributing
the kernel is a solo project but device drivers (maybe even userspace applications???) are welcome

# License
Definitions:
'Software' is any document distributed along this License
'Plausibly alterated' stands to mean any document that may be be used to re-generate the Software through a computer program employing the reverse logic
of the one that generated the document, meaning that all manner off transpilation into new "source code" that would have otherwise been exempt from this License
is not allowed under the terms of this License.

You may distirbute the non-Plausibly alterated products of the Software such as runnable binaries, possibly for a fee
but not the Software or its Plausibly alterated products.
You may run products of the Software.
