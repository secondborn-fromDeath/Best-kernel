/*
This has all of the interrupt routines
*/
namespace OS_INTERRUPTS{
//page 3218
#define SYSCALL 0
#define NMI 2
#define BREAKPOINT 3
#define ILLEGAL_INSTRUCTION 6
#define DEVICE_NOT_AVAILABLE 7
#define DOUBLE_FAULT 8
#define MISSING_SEGMENT 11
#define STACK_FAULT 12
#define GPE 13
#define PAGE_FAULT 14
#define FPE 16
#define ALIGNMENT 17		//unused, see init/setup_MSRS.cpp
#define MACHINE_CHECK 18
#define SIMD_FPE 19

//following are raw interrupts
//32+... boot interrupts
#define B(x) &templ_receive_info(x)
enum{ base = 47, FIRST_SIPI, REQUEST_DATA_POINTER, NOTIFY_DATA, ENDOF_TRANSACTION};
//52+ os runtime interrupts
enum{SLEEP = 52,HIGHPRIO_SLEEP,};
#define WAKE 255

/*
HANDLERS:
for now i am using killer() a lot but when logging comes in it wont be that way
*/
constexpr auto * interrupt_routines[] = [\
&syscall,,\
&nmi_enter,,\
&breakpoint_,,,\
&killer,\
&killer,\
&double_fault_handler,,,\
&killer,\
&killer,\
&killer,\
&pagefault_handler,\
&fpe,,\
&acpi_machine_check,\
&simd_fpe,,,,,,,,,,,,,,\//32 not covered
B(0),B(1),B(2),B(3),B(4),B(5),B(6),B(7),B(8),B(9),B(10),B(11),B(12),B(13),B(14),B(15),\
&sipi,\
&serve_data,\
&receive_data,\
&boot_wake,\
&halt,\
&highprio_halt,\//wake is vector 255
];




};
