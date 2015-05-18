#include <inc/signal.h>

extern int sys_signal(envid_t envid, sig_t signo, void * handler);
extern int sys_kill(envid_t envid, sig_t signal);
extern struct Env * thisenv;

int 
signal(sig_t signo, void (* handler)(sig_t sig))
{
	return sys_signal(thisenv->env_id, signo, (void *)handler);
}

int 
kill(envid_t envid, sig_t signal)
{
	return sys_kill(envid, signal);
}