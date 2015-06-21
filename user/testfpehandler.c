#include <inc/lib.h>

int zero;

void
handler(sig_t signo)
{
	static int i = 10;
	zero = 1;
	cprintf("ooooops!, %d\n", i);
	if (!(--i)) {
		exit();
	}
}

void
umain(int argc, char **argv)
{
	sys_page_alloc(0, (void *)(UXSTACKTOP - PGSIZE), PTE_W | PTE_U);
    signal(SIGFPE, handler);
	zero = 0;
	cprintf("1/0 is %08x!\n", 1/zero);
}

