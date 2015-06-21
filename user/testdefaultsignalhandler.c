#include <inc/lib.h>

void
umain(int argc, char ** argv)
{
	sys_page_alloc(0, (void *)(UXSTACKTOP - PGSIZE), PTE_W | PTE_U);
	kill(0, SIGUSR1);
}

