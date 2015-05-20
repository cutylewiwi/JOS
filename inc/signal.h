/* See COPYRIGHT for copyright information. */

#ifndef JOS_INC_SIGNAL_H
#define JOS_INC_SIGNAL_H

#include <inc/types.h>
#include <inc/env.h>

#define SIGNALCOUNT 13

typedef int32_t sig_t;
typedef int32_t sigset_t;

enum {
	SIGINT = 0,
	SIGQUIT,
	SIGILL,
	SIGFPE,
	SIGKILL,
	SIGUSR1,
	SIGSEGV,
	SIGUSR2,
	SIGALARM,
	SIGTERM,
	SIGCONT,
	SIGSTOP,
	SIGTSTP,
	NSYGNAL
};

enum {
	SIG_BLOCK = 0x0FFFFFFF,
	SIG_UNBLOCK,
	SIG_SETMASK,
	NSIGHOW
};

int signal(sig_t signo, void (* handler)(sig_t sig));
int kill(envid_t envid, sig_t signal);
int sigprocmask(int how, const sigset_t *set, sigset_t *oldset); 
int sigemptyset(sigset_t *set);
int sigfillset(sigset_t *set);
int sigaddset(sigset_t *set, int signum);
int sigdelset(sigset_t *set, int signum);
int sigismember(const sigset_t *set, int signum);

#ifndef __ASSEMBLER__

struct SignalUTrapframe {
	uint32_t utf_sysno;
	uint32_t utf_handler;
	struct PushRegs utf_regs;
	uint32_t utf_eip;
	uint32_t utf_eflags;
	uint32_t utf_esp;
};

#endif	/* !__ASSEMBLER__ */

#endif /* !JOS_INC_SIGNAL_H */