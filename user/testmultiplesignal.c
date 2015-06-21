#include <inc/lib.h>

void
handler1(sig_t signo)
{
	cprintf("this is signal handler for signo: %d\n", signo);
	exit();
}


void
handler2(sig_t signo)
{
	cprintf("this is signal handler for signo: %d\n", signo);
}


void
umain(int argc, char ** argv)
{
	int r;

	signal(SIGUSR1, handler1);
	signal(SIGUSR2, handler2);

	if (!(r = fork())) {
		while (1){
			cprintf("yield!\n");
			sys_yield();
		}
	}
	else if (r > 0) {
		sys_env_set_status(r, ENV_NOT_RUNNABLE);
		kill(r, SIGUSR1);
		kill(r, SIGUSR2);
		sys_env_set_status(r, ENV_RUNNABLE);
		sys_yield();
		sys_yield();
		sys_yield();
		sys_yield();
		sys_yield();
	}
}