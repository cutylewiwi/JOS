/* See COPYRIGHT for copyright information. */

#ifndef JOS_INC_SIGNAL_H
#define JOS_INC_SIGNAL_H

#include <inc/types.h>
#include <inc/env.h>

#define SIGNALCOUNT 32
#define SIGIGN	0xffffffff
#define SIGABT	0xfffffffe
#define SIGNOR	0xfffffffd

typedef int32_t sig_t;

enum {
	SIGHUB = 0,
	SIGINT,
	SIGUSR1,
	SIGUSR2,
	NSYGNAL
};

int signal(sig_t signo, void * handler);
int kill(envid_t envid, sig_t signal);

#endif /* !JOS_INC_SIGNAL_H */