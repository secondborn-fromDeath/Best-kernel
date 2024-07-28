# Best-kernel
better than even loonix.

# Features
Drivers run on ring 3 and interface with devices through system calls, supports driver hotplugging ("protocol" "drivers" included), *most* drivers can omitted at boot using a simple configuration file, same goes for settings such as the maximum file descriptors a process may hold or the number of threads for a process.
At boot the kernel is loaded directly and performs the initialization duties, so you dont need to waste disk space with a huge bootloader.

# Support
UEFI x64

# Updates
may take anywhere between a day and weeks depending on what it is I have implemented.

# Contributing
the kernel is a solo project but device drivers (maybe even userspace applications???) are welcome

# License
Definitions:
'Software' is any document distributed along this License
'Plausibly alterated' stands to mean any document that may be be used to re-generate the Software through a computer program employing the reverse logic
of the one that generated the document, meaning that all manner off transpilation into new "source code" that would have otherwise been exempt from this License is not allowed under the terms of this License.

You may distribute the non-Plausibly alterated products of the Software such as runnable binaries, possibly for a fee
but not the Software or its Plausibly alterated products.
You may run products of the Software.

This License will be applied to the full extent of American law.
