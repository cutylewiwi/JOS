#include <inc/lib.h>


void 
signal_handler()
{
	static int i = 0;
	cprintf("this is a signal handler: envid: %08x\n", thisenv->env_id);
	if ((i++) == 3){
		exit();
	}
}

void
umain(int argc, char ** argv)
{
	envid_t child;
	
	//sys_page_alloc(0, (void*) (UXSTACKTOP - PGSIZE), PTE_P|PTE_U|PTE_W);

	signal(SIGUSR1, signal_handler);

	cprintf("shandler: %p\n", signal_handler);
	//cprintf("sshandler: %p\n", signal_handler());

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
			cprintf("yield!\n");
		}
	}
}