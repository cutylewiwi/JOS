#include <inc/lib.h>

void
handler1(sig_t signo)
{
	static int i = 0;
	cprintf("SIGUSR1 i: %d\n", i);

	if ((i++) != 3) {
		kill(0, SIGUSR2);
	}

	cprintf("SIGUSR1 END\n");
}

void
handler2(sig_t signo)
{
	cprintf("SIGUSR2\n");
	kill(0, SIGUSR1);
	cprintf("SIGUSR2 END!\n");
}

void
umain(int argc, char **argv)
{
	sys_page_alloc(0, (void *)(UXSTACKTOP - PGSIZE), PTE_W | PTE_U);
	signal(SIGUSR1, handler1);
	signal(SIGUSR2, handler2);
	
	kill(0, SIGUSR1);
}