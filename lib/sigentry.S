#include <inc/mmu.h>
#include <inc/memlayout.h>
#include <inc/trap.h>

// extern int sys_after_signal_handler(void * returnAddress, struct PushRegs * savedRegs);

// exception stack:
//
//	saved esp
//	saved eflags
//	saved eip
//	saved reg_eax
//	...
//	saved reg_esi
//	saved reg_edi				
//	signal handler
//	signal no. 			<-- %esp

.text
.global _signal_upcall
_signal_upcall:
	// invoke handler
	movl 4(%esp), %eax
	call *%eax

	// Para. for sys_after_signal_handler
	push %esp;

	// now we need turn back to kernel mode
	//mov sys_after_signal_handler, %eax
	call sys_after_signal_handler
