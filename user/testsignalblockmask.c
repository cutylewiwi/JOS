#include <inc/lib.h>

void
handler(int sig)
{
	cprintf("This is a handler!\n");
}

void
umain(int argc, char ** argv)
{
	sigset_t set = 0;
	sys_page_alloc(0, (void *)(UXSTACKTOP - PGSIZE), PTE_W | PTE_U);
	signal(SIGUSR1, handler);

	kill(0, SIGUSR1);

	sigaddset(&set, SIGUSR1);
	sigprocmask(SIG_BLOCK, &set, NULL);

	kill(0, SIGUSR1);

}
