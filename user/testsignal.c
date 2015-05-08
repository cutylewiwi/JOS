#include <inc/lib.h>


void 
signal_handler()
{
	cprintf("this is a signal handler: envid: %08x\n", thisenv->env_id);
	exit();
}

void
umain(int argc, char ** argv)
{
	envid_t child;

	signal(SIGUSR1, signal_handler);

	if ((child = fork()) < 0) {
		cprintf("ooooops!\n");
	}
	else if (!child) {
		cprintf("this is child: %08x, sending signal to parent: %08x\n", thisenv->env_id, thisenv->env_parent_id);
		kill(thisenv->env_parent_id, SIGUSR1);
	}
	else {
		while(1) {
			sys_yield();
		}
	}
}