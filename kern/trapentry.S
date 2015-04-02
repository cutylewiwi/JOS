/* See COPYRIGHT for copyright information. */

#include <inc/mmu.h>
#include <inc/memlayout.h>
#include <inc/trap.h>



###################################################################
# exceptions/interrupts
###################################################################

/* TRAPHANDLER defines a globally-visible function for handling a trap.
 * It pushes a trap number onto the stack, then jumps to _alltraps.
 * Use TRAPHANDLER for traps where the CPU automatically pushes an error code.
 *
 * You shouldn't call a TRAPHANDLER function from C, but you may
 * need to _declare_ one in C (for instance, to get a function pointer
 * during IDT setup).  You can declare the function with
 *   void NAME();
 * where NAME is the argument passed to TRAPHANDLER.
 */
#define TRAPHANDLER(name, num)						\
	.globl name;		/* define global symbol for 'name' */	\
	.type name, @function;	/* symbol type is function */		\
	.align 2;		/* align function definition */		\
	name:			/* function starts here */		\
	pushl $(num);							\
	jmp _alltraps

/* Use TRAPHANDLER_NOEC for traps where the CPU doesn't push an error code.
 * It pushes a 0 in place of the error code, so the trap frame has the same
 * format in either case.
 */
#define TRAPHANDLER_NOEC(name, num)					\
	.globl name;							\
	.type name, @function;						\
	.align 2;							\
	name:								\
	pushl $0;							\
	pushl $(num);							\
	jmp _alltraps


/*
 * lab 3 challenge 1
 */

#define TABLE_NOEC(name, num)				\
.data;										\
	.long name;								\
.text;										\
	TRAPHANDLER_NOEC(name, num)

#define TABLE_EC(name, num)					\
.data;										\
	.long name;								\
.text; 										\
	TRAPHANDLER(name, num)


.data
	.globl trapentry_idt
	.align 2
trapentry_idt:
	TABLE_NOEC(Handler0, 0)
	TABLE_NOEC(Handler1, 1)
	TABLE_NOEC(Handler2, 2)
	TABLE_NOEC(Handler3, 3)
	TABLE_NOEC(Handler4, 4)
	TABLE_NOEC(Handler5, 5)
	TABLE_NOEC(Handler6, 6)
	TABLE_NOEC(Handler7, 7)
	TABLE_EC(Handler8, 8)
.data
	.long 0					/* trap 9 */
	TABLE_EC(Handler10, 10)
	TABLE_EC(Handler11, 11)
	TABLE_EC(Handler12, 12)
	TABLE_EC(Handler13, 13)
	TABLE_EC(Handler14, 14)
.data
	.long 0					/* trap 15 */
	TABLE_NOEC(Handler16, 16)
	TABLE_EC(Handler17, 17)
	TABLE_NOEC(Handler18, 18)
	TABLE_NOEC(Handler19, 19)
.data
	.space 112
	TABLE_NOEC(System_call, 48)

.text

/*
 * Lab 3: Your code here for generating entry points for the different traps.
 */

/*
 * Lab 3: Your code here for _alltraps
 */

_alltraps:
	pushl %ds
	pushl %es
	pushal
	pushl %esp
	movw $GD_KD, %ax 
	movw %ax, %ds
	movw %ax, %es
	call trap
