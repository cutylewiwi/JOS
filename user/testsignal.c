#include <inc/lib.h>


void 
signal_handler(sig_t signo)
{
	static int i = 0;
	cprintf("this is a signal handler: envid: %08x\ni:%d\nsigno: %d\n", thisenv->env_id, i, signo);
	if ((i++) == 3){
		exit();
	}
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
		//sys_yield();
		//sys_yield();
		cprintf("this is child: %08x, sending signal to parent: %08x\n", thisenv->env_id, thisenv->env_parent_id);
		kill(thisenv->env_parent_id, SIGUSR1);
		sys_yield();
		//cprintf("kill\n");
		kill(thisenv->env_parent_id, SIGUSR1);
		sys_yield();
		//cprintf("kill\n");
		kill(thisenv->env_parent_id, SIGUSR1);
		sys_yield();
		//cprintf("kill\n");
		kill(thisenv->env_parent_id, SIGUSR1);
		sys_yield();
	}
	else {
		while(1) {
			sys_yield();
			cprintf("yield!\n");
		}
	}
}