// Called from entry.S to get us going.
// entry.S already took care of defining envs, pages, uvpd, and uvpt.

#include <inc/lib.h>

extern void umain(int argc, char **argv);
extern int sys_set_signal_upcall(envid_t envid, void * upcall);

const volatile struct Env *thisenv;
const char *binaryname = "<unknown>";

struct Env *
getThisenv()
{
	return (struct Env *)(envs + ENVX(sys_getenvid()));
}

void
libmain(int argc, char **argv)
{
	// set thisenv to point at our Env structure in envs[].
	// LAB 3: Your code here.
	thisenv = getThisenv();

	// save the name of the program so that panic() can use it
	if (argc > 0)
		binaryname = argv[0];

	extern void _signal_upcall(void);
	cprintf("upcall: 0x%08x\n", _signal_upcall);
	if (sys_set_signal_upcall(0, _signal_upcall) < 0) {
		cprintf("signal init failed!\n");
		exit();
	}

	// call user main routine
	umain(argc, argv);

	// exit gracefully
	exit();
}

