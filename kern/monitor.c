// Simple command-line kernel monitor useful for
// controlling the kernel and exploring the system interactively.

#include <inc/stdio.h>
#include <inc/string.h>
#include <inc/memlayout.h>
#include <inc/assert.h>
#include <inc/x86.h>
#include <inc/attributed.h>

#include <kern/console.h>
#include <kern/monitor.h>
#include <kern/kdebug.h>
#include <kern/trap.h>
#include <kern/env.h>

#ifdef MAPPINGDEBUG
#include <kern/pmap.h>
#include <inc/mmu.h>
#endif

#define CMDBUF_SIZE	80	// enough for one VGA text line


struct Command {
	const char *name;
	const char *desc;
	// return -1 to force monitor to exit
	int (*func)(int argc, char** argv, struct Trapframe* tf);
};

static struct Command commands[] = {
	{ "help", "Display this list of commands", mon_help },
	{ "kerninfo", "Display information about the kernel", mon_kerninfo },
	{ "backtrace", "Display the trace of the stack", mon_backtrace },
#ifdef MAPPINGDEBUG
	{ "showmappings", "Display the mapping for the physical memory address", mon_showmappings },
	{ "chpermissions", "change the permission of mapping to virtual address", mon_chpermissons },
	{ "coredump", "core dump", mon_coredump},
#endif
	{ "cutytest", "cuty-lewiwi needs to test some functions", mon_cutytest},
	{ "continue", "continue execution", mon_continue},
	{ "c", "continue execution", mon_continue},
	{ "stepi", "single step instruction", mon_singlestep},
	{ "si", "single step instruction", mon_singlestep}
};
#define NCOMMANDS (sizeof(commands)/sizeof(commands[0]))

/***** Implementations of basic kernel monitor commands *****/

int
mon_help(int argc, char **argv, struct Trapframe *tf)
{
	int i;

	for (i = 0; i < NCOMMANDS; i++)
		cprintf("%s - %s\n", commands[i].name, commands[i].desc);
	return 0;
}

int
mon_kerninfo(int argc, char **argv, struct Trapframe *tf)
{
	extern char _start[], entry[], etext[], edata[], end[];

	cprintf("Special kernel symbols:\n");
	cprintf("  _start                  %08x (phys)\n", _start);
	cprintf("  entry  %08x (virt)  %08x (phys)\n", entry, entry - KERNBASE);
	cprintf("  etext  %08x (virt)  %08x (phys)\n", etext, etext - KERNBASE);
	cprintf("  edata  %08x (virt)  %08x (phys)\n", edata, edata - KERNBASE);
	cprintf("  end    %08x (virt)  %08x (phys)\n", end, end - KERNBASE);
	cprintf("Kernel executable memory footprint: %dKB\n",
		ROUNDUP(end - entry, 1024) / 1024);
	return 0;
}

int
mon_backtrace(int argc, char **argv, struct Trapframe *tf)
{
	// Your code here.
	int *ebp = (int *)read_ebp();
	struct Eipdebuginfo info;
	cprintf("Stack backtrace:\n");
	do {
		debuginfo_eip(ebp[1], &info);
		cprintf("  ebp %08x  eip %08x  args %08x %08x %08x %08x %08x\n", 
			(int *)ebp, ebp[1], ebp[2], ebp[3], ebp[4], ebp[5], ebp[6]);
		cprintf("         %s:%d: %.*s+%d\n", 
			info.eip_file, info.eip_line, info.eip_fn_namelen, info.eip_fn_name, ebp[1] - info.eip_fn_addr);
	} while ((ebp = (int *)(*ebp)) != NULL);

	return 0;
}


// use to test some functions

//#define PGSIZE (1<<12)

void test(void *va, int len)
{
	cprintf("va: %p, len: %d\n", va, len);
	cprintf("va: %p\n", va);
	va = ROUNDDOWN(va, PGSIZE);
	cprintf("rounddown: %p\n", va);
	cprintf("va+len: %p\n", va+len);
	void * end = ROUNDUP(va+len, PGSIZE);
	cprintf("roundup: %p\n", end);
}

int
mon_cutytest(int argc, char **argv, struct Trapframe *tf)
{
	cprintf("cutytest:\n");
	//cprintf("%d %d %d\n", COLOR_BLUE, COLOR_GREEN, COLOR_RED);
	//cprintf("%m%s\n%m%s\n%m%s\n", COLOR_BLUE, "blue", COLOR_GREEN, "green", COLOR_RED, "red");
	//int x = 1, y = 3, z = 4;
	//cprintf("x %d, y %x, z %d\n", x, y, z);
	//printpage((void *)my_atoi("0xf0000000"), my_atoi("0xf0000000"));
	test((void *)0x0, 10);
	test((void *)0x2000, 10);
	test((void *)0x2000, PGSIZE);
	test((void *)0x2001, PGSIZE-1);
	test((void *)0x2001, PGSIZE-2);
	test((void *)0x2001, 2*PGSIZE);
	test((void *)0xfffff000, 10);

	//cprintf("%s: %d\n%s: %d\n%s: %d\n", 
	//	"090", my_atoi("090"), "0x10", my_atoi("0x10"), "10", my_atoi("10"));
    cprintf("\n");
	return 0;
}

// continue in debug mode
int
mon_continue(int argc, char **argv, struct Trapframe *tf)
{
	unsigned int eflags;
	if (tf && (tf->tf_trapno == T_BRKPT || tf->tf_trapno == T_DEBUG)) {
		tf->tf_eflags &= ~(1 << 8);
		env_run(curenv);
	}
	else{
		cprintf("<%s>: not in breaking point mode!\n", *argv);
	}
	return 0;
}


// single step instruction mode
int
mon_singlestep(int argc, char **argv, struct Trapframe *tf)
{
	if (tf && (tf->tf_trapno == T_BRKPT || tf->tf_trapno == T_DEBUG)) {
		tf->tf_eflags |= (1 << 8);
		env_run(curenv);
	}
	return 0;	
}

#ifdef MAPPINGDEBUG
// showmapping for lab 2 challenge
int
mon_showmappings(int argc, char **argv, struct Trapframe *tf)
{
	int i;
	void * va, * end;
	struct PageInfo * mapping;
	pte_t * pte;

	if (argc != 3){
		cprintf("usage: <%s> -va_start -va_end\n", argv[0]);
		return 0;
	}
	cprintf("VA\t\tPA\t\tPERMISSIONS\n");
	
	va = (void *)ROUNDDOWN((char *)my_atoi(argv[1]), PGSIZE);
	end = (void *)my_atoi(argv[2]);
	for (; end - va >= PGSIZE; va += PGSIZE){
		mapping = page_lookup(kern_pgdir, va, &pte);
		
		if (mapping){
			cprintf("%p\t%p\t", va, page2pa(mapping));
			for (i = 0; i < 12; i++){
				if ((1 << (11 - i)) & *pte){
					cprintf("%c","---GSDACTUWP"[i]);
				}
				else{
					cprintf("-");
				}
			}
			cprintf("\n");
		}
		else{
			cprintf("%p\tNULL\t------------\n", va);
		}
	}

	return 0;
}

// change permissions for lab 2 challenge
int
mon_chpermissons(int argc, char **argv, struct Trapframe *tf)
{
	pte_t * pte;
	void * va;
	int permissions;

	if (argc != 3){
		cprintf("usage: <%s> -va -permissions\n", argv[0]);
		return 0;
	}

	va = (void *)ROUNDDOWN((char *)my_atoi(argv[1]), PGSIZE);
	pte = pgdir_walk(kern_pgdir, va, 0);
	permissions = my_atoi(argv[2]) & 0xFFF;
	*pte |= permissions;
	return 0;
}

// core dump for lab 2 challenge
int 
mon_coredump(int argc, char **argv, struct Trapframe *tf)
{
	void * va;
	unsigned int page_start, page_end, i;
	volatile pte_t * pte;
	int flag;
	if (argc != 4 || (argv[1][1] != 'p' && argv[1][1] != 'v')){
		cprintf("usage: <%s> -p/v -va_start -va_end\n", argv[0]);
		return 0;
	}

	page_start = (unsigned int)ROUNDDOWN((char *)my_atoi(argv[2]), PGSIZE);
	page_end = (unsigned int)ROUNDUP((char *)my_atoi(argv[3]), PGSIZE);

	if (argv[1][1] == 'p') {
		cprintf("Physical Address:\n");
	}
	else {
		cprintf("Virtual Address:\n");
	}

	for (i = page_start; i <= page_end; i += PGSIZE){
		if (argv[1][1] == 'v'){
			va = (void *)i;
			pte = pgdir_walk(kern_pgdir, va, 0);
			
			if (!(pte && (*pte & PTE_P))) {
				cprintf("no mapping at virtual page: %p\n", (void *)i);
				continue;
			}
		}
		else{
			va = pa2va(i, &flag);
			if (!flag){
				cprintf("no mapping at physical page: %p\n", (void *)i);
				continue;
			}
		}

		printpage(va, i);
	}

	return 0;
}

void *
pa2va(physaddr_t pa, int *flag)
{
	int i;
	void * va;
	pte_t * pte;
	*flag = 0;
	for (i = 0, va = (void *)0; i < (1 << 20); i++, va += PGSIZE){
		pte = pgdir_walk(kern_pgdir, va, 0);
		if (pte && (*pte & PTE_P) && PTE_ADDR(*pte) == (pa & ~0xFFF)){
			*flag = 1;
			break;
		}
	}
	return va;
}

void printpage(void * va, int start)
{
	void * end;
	unsigned int * ptr;
	int i;
	char c;
	void * hold = va;
	cprintf("%p\n", va + PGSIZE - 1);
	for (end = va + PGSIZE - 1; va <= end; va += 4 * sizeof(int)){
		ptr = (unsigned int *)va;
		cprintf("%p ", (void *)(start + va - hold));
		cprintf("0x%08x ", *ptr);
		cprintf("0x%08x ", *(ptr+1));
		cprintf("0x%08x ", *(ptr+2));
		cprintf("0x%08x ", *(ptr+3));
		for (i = 0; i < 16; i++){
			c = *(char *)(va+i);
			if (c <= 32){
				cprintf("·");
			}
			else{
				cprintf("%c", c);
			}
		}
		cprintf("\n");
	}
}

int 
my_atoi(char * input)
{
	int result = 0;
	int mode = 0;
	// 0: origin
	// 1: first 0
	// 2: 16 base or 8 base
	// 3: 10 base
	int base = 10;
	int delta = 0;
	char * origin = input;
	while (*input){
		switch (*input){
			case '0':
				if (!mode){
					mode = 1;
				}
				break;
			case 'x':
				if (mode == 1){
					mode = 2;
					base = 16;
				}
				break;
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
				if (mode == 1){
					mode = 2;
					base = 8;
				}
				// fall through
			case '8':
			case '9':
				delta = (*input) - '0';
				if (!mode){
					mode = 3;
					base = 10;
				}
				break;
			case 'a':
			case 'b':
			case 'c':
			case 'd':
			case 'e':
			case 'f':
				delta = (*input) - 'a' + 10;
				if (mode == 1){
					mode = 2;
					base = 16;
				}
				break;
			case 'A':
			case 'B':
			case 'C':
			case 'D':
			case 'E':
			case 'F':
				delta = (*input) - 'A' + 10;
				if (mode == 1){
					mode = 2;
					base = 16;
				}
				break;
			default:
				cprintf("my_atoi: cant recognized input: %s\n", origin);
				return -1;
		}
		if (delta >= base){
			cprintf("my_atoi: illegal input string!\n");
			return -1;
		}
		result = result * base + delta;
		delta = 0;
		input ++;
	}
	return result;
}

#endif

/***** Kernel monitor command interpreter *****/

#define WHITESPACE "\t\r\n "
#define MAXARGS 16

static int
runcmd(char *buf, struct Trapframe *tf)
{
	int argc;
	char *argv[MAXARGS];
	int i;

	// Parse the command buffer into whitespace-separated arguments
	argc = 0;
	argv[argc] = 0;
	while (1) {
		// gobble whitespace
		while (*buf && strchr(WHITESPACE, *buf))
			*buf++ = 0;
		if (*buf == 0)
			break;

		// save and scan past next arg
		if (argc == MAXARGS-1) {
			cprintf("Too many arguments (max %d)\n", MAXARGS);
			return 0;
		}
		argv[argc++] = buf;
		while (*buf && !strchr(WHITESPACE, *buf))
			buf++;
	}
	argv[argc] = 0;

	// Lookup and invoke the command
	if (argc == 0)
		return 0;
	for (i = 0; i < NCOMMANDS; i++) {
		if (strcmp(argv[0], commands[i].name) == 0)
			return commands[i].func(argc, argv, tf);
	}
	cprintf("Unknown command '%s'\n", argv[0]);
	return 0;
}

void
monitor(struct Trapframe *tf)
{
	char *buf;

	cprintf("Welcome to the JOS kernel monitor!\n");
	cprintf("Type 'help' for a list of commands.\n");

	if (tf != NULL)
		print_trapframe(tf);

	while (1) {
		buf = readline("K> ");
		if (buf != NULL)
			if (runcmd(buf, tf) < 0)
				break;
	}
}
