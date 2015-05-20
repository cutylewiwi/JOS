#include <inc/signal.h>

extern int sys_signal(envid_t envid, sig_t signo, void * handler);
extern int sys_kill(envid_t envid, sig_t signal);
extern int sys_get_env_signal_blocked(envid_t envid);
extern int sys_set_env_signal_blocked(envid_t envid, sigset_t sigset);
extern struct Env * thisenv;

int 
signal(sig_t signo, void (* handler)(sig_t sig))
{
	return sys_signal(0, signo, (void *)handler);
}

int 
kill(envid_t envid, sig_t signal)
{
	return sys_kill(envid, signal);
}

int 
sigprocmask(int how, const sigset_t *set, sigset_t *oldset)
{
	int r = 0;
	sigset_t old = sys_get_env_signal_blocked(0);
	if (how == SIG_BLOCK) {
		r = sys_set_env_signal_blocked(0, old | *set);
	}
	else if (how == SIG_UNBLOCK) {
		r = sys_set_env_signal_blocked(0, old & ~(*set));
	}
	else if (how == SIG_SETMASK) {
		r = sys_set_env_signal_blocked(0, *set);
	}

	if (oldset) {
		*oldset = old;
	}
	return 0;
}

int 
sigemptyset(sigset_t *set)
{
	*set = 0;
	return 0;
}

int
sigfillset(sigset_t *set)
{
	*set = 0xFFFFFFFF;
	return 0;
}

int
sigaddset(sigset_t *set, int signum)
{
	*set |= (1 << signum);
	return 0;
}

int
sigdelset(sigset_t *set, int signum)
{
	int ret;
	if (*set & (1 << signum)) {
		*set &= ~(1 << signum);
		ret = 0;
	}
	else {
		ret = -1;
	}

	return ret;
}

int
sigismember(const sigset_t *set, int signum)
{
	int ret = *set & (1 << signum);
	return !!ret;
}