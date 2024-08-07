#these are to be called by functions in sched.cpp


store_state:#							this not preserving C callconv is fine (compiler wasting push and pop instructions is whatever srsly)
MOVQ	%%rsp,%%cr14
MOVQ	%%rax,%%cr12
JMP	+0
MOVQ	QWORD PTR (%%rsp),%%rax#				thread pointer
POPF	%%rcx
MOVQ	%%rcx,thread.state.flags(%%rax)
MOVQ	%%rax,%%rsp
MOVQ	%%cr12,%%rax
MOVQ	%%rax,QWORD PTR thread.state.accumulator(%%rsp)
MOVQ	%%rcx,QWORD PTR thread.state.counter(%%rsp)
MOVQ	%%rdx,QWORD PTR thread.state.rdx(%%rsp)
MOVQ	%%rbx,QWORD PTR thread.state.rbx(%%rsp)
MOVQ	%%cr14,%%rax
MOVQ	%%rax,QWORD PTR thread.state.stack_pointer(%%rsp)
MOVQ	%%rdi,QWORD PTR thread.state.string_dest(%%rsp)
MOVQ	%%rsi,QWORD PTR thread.state.string_source(%%rsp)
MOVQ	%%rbp,QWORD PTR thread.state.frame_pointer(%%rsp)
MOVQ	%%r8,QWORD PTR thread.state.eight(%%rsp)
MOVQ	%%r9,QWORD PTR thread.state.nine(%%rsp)
MOVQ	%%r10,QWORD PTR thread.state.ten(%%rsp)
MOVQ	%%r11,QWORD PTR thread.state.eleven(%%rsp)
MOVQ	%%r12,QWORD PTR thread.state.twelve(%%rsp)
MOVQ	%%r13,QWORD PTR thread.state.thirteen(%%rsp)
MOVQ	%%r14,QWORD PTR thread.state.fourteen(%%rsp)
MOVQ	%%r15,QWORD PTR thread.state.fifteen(%%rsp)
MOVQ	QWORD PTR interrupt_stack.instruction_pointer(%%rax),%%rcx
MOVQ	QWORD PTR thread.state.instruction_override(%%rsp),%%rdx
TEST	%%rdx,%%rdx
JZ 	ststate_load_ip
MOVQ	rcx,QWORD PTR thread.state.instruction_override(%%rsp)
JMP	endof_instructions_query
ststate_load_ip:
MOVQ	rcx,QWORD PTR thread.state.instruction_pointer(%%rsp)
endof_instruction_query:
EXCHG	%%rax,%%rsp
RET#								returns the thread pointer

enter_ringzero:
MOVQ	%%rax,%%cr14
JMP	+0
MOVQ	$14,%%rax#			fast task priority
MOVQ	%%rax,%%cr8
JMP	+0
MOVQ	%%cr13,%%rax#			ringzero pagetree
MOVQ	%%rax,%%cr3#			loading pagetree
JMP	+0
MOVQ	%%cr11,%%rax#			thread pointer
MOVQ	%%rax,%%cr12
SUBQ	$8,%%rsp
PUSHQ	%%rax#				passing for store_state
MOVQ	%%cr14,%%rax
CALL	store_state
RET

leave_ringzero:
MOVQ	8(%%rsp),%%rax#								thread pointer
MOVQ	%%cr8,%%rcx
MOVQ	%%rcx,%%cr3
JMP	+0#									constructing the interrupt return stack
XORQ	%%edx,%%edx#								stack segment
PUSHQ	%%rdx
MOVQ	QWORD PTR thread.state.stack_pointer(%%rax),%%rdx#			stack pointer
PUSHQ	%%rdx
XORQ	%%edx,%%edx#								code segment
PUSHQ	%%rdx
MOVQ	QWORD PTR thread.state.instruction_override(%%rax),%%rdx#		instruction pointer
TEST	%%rdx,%%rdx
JNZ	lvrin_overridden
MOVQ	QWORD PTR thread.state.instructioin_pointer(%%rax),%%rdx
lvrin_overriden:
PUSHQ	%%rdx#									^^^
MOVQ	thread.state.flags(%%rsp),%%rdx#					flags register
PUSHQ	%%rdx

MOVQ	%%rsp,%%cr14
JMP	+0
MOVQ	%%rax,%%rsp#								rsp now the thread pointer
MOVQ	QWORD PTR thread.state.accumulator(%%rsp),%%rax
MOVQ	QWORD PTR thread.state.counter(%%rsp),%%rcx
MOVQ	QWORD PTR thread.state.rdx(%%rsp),%%rdx
MOVQ	QWORD PTR thread.state.rbx(%%rsp),%%rbx
MOVQ	QWORD PTR thread.state.string_dest(%%rsp),%%rdi
MOVQ	QWORD PTR thread.state.string_source(%%rsp),%%rsi
MOVQ	QWORD PTR thread.state.frame_pointer(%%rsp),%%rbp
MOVQ	QWORD PTR thread.state.eight(%%rsp),%%r8
MOVQ	QWORD PTR thread.state.nine(%%rsp),%%r9
MOVQ	QWORD PTR thread.state.ten(%%rsp),%%r10
MOVQ	QWORD PTR thread.state.evelen(%%rsp),%%r11
MOVQ	QWORD PTR thread.state.twelve(%%rsp),%%r12
MOVQ	QWORD PTR thread.state.thirteen(%%rsp),%%r13
MOVQ	QWORD PTR thread.state.fourteen(%%rsp),%%r14
MOVQ	QWORD PTR thread.state.fifteen(%%rsp),%%r15
MOVQ	%%cr14,%%rsp
IRETQ#										loading sp and ip through iretq






