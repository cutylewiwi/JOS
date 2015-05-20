

#include <inc/x86.h>
#include <inc/error.h>
#include <inc/string.h>
#include <inc/assert.h>

#include <kern/env.h>
#include <kern/pmap.h>
#include <kern/trap.h>
#include <kern/syscall.h>
#include <kern/console.h>
#include <kern/sched.h>
#include <kern/signal.h>


#define SIGNALIGNORE 0xFFFFFFFF
#define SIGNALTERMINATE 0xFFFFFFFE
#define SIGNALSTOP 0xFFFFFFFD
#define SIGNALCONT 0xFFFFFFFC

const static int defaultHandler[SIGNALCOUNT] = {
	SIGNALTERMINATE,
	SIGNALTERMINATE,
	SIGNALTERMINATE,
	SIGNALTERMINATE
};

const static char * signalName[SIGNALCOUNT] = {
	"SIGHUB",
	"SIGINT",
	"SIGUSR1",
	"SIGUSR2",
	"NSYGNAL"
};

int 
signal_kill(envid_t envid, sig_t signal)
{
	struct Env * e;
	int r;

	if ((r = envid2env(envid, &e, 0)) < 0) {
		return r;
	}

	e->env_signal_pending |= (1 << signal);

	return 0;
}

int 
set_signal_handler(envid_t envid, sig_t signal, void* handler)
{
	struct Env *e;
	int r;

	if ((r = envid2env(envid, &e, 0)) < 0) {
		return r;
	}

	if (!handler) {
		return -1;
	}

	e->env_signal_handlers[signal] = handler;

	return 0;
}

static int 
signal_terminate(struct Env * env, sig_t signal)
{
	cprintf("env [%08x] terminate by signal %s\n", env->env_id, signalName[signal]);
	//panic("sigabort");
	env_destroy(env);
	return 0;
}

static int
signal_ignore(struct Env * env, sig_t signal)
{
	cprintf("env [%08x] ignores signal %s\n", env->env_id, signalName[signal]);

	return 0;
}

static int
signal_stop(struct Env * env, sig_t signal)
{
	cprintf("env [%08x] stoped by signal %s\n", env->env_id, signalName[signal]);

	env->env_status = ENV_NOT_RUNNABLE;

	return 0;
}

static int
signal_cont(struct Env * env, sig_t signal)
{
	cprintf("env [%08x] continued by signal %s\n", env->env_id, signalName[signal]);

	env->env_status = ENV_RUNNABLE;

	return 0;
}

void
signal_handle(envid_t envid, sig_t signal)
{
	struct Env * e;
	int r;
	void * handler;
	struct SignalUTrapframe * utrapframe;

	if ((r = envid2env(envid, &e, 0)) < 0) {
		panic("signal_handle: failed: %e", r);
	}

	e->env_signal_pending &= ~(1 << signal);
	e->env_signal_blocked |= (1 << signal);

	if (signal == SIGINT) {
		cprintf("SIGINT!\n");
		e->env_signal_blocked &= ~(1 << signal);
		return;
	}

	if (e->env_signal_handlers[signal]) {
		if (e->env_tf.tf_esp >= UXSTACKTOP - PGSIZE
			&& e->env_tf.tf_esp < UXSTACKTOP) {
			utrapframe = (struct SignalUTrapframe *)(e->env_tf.tf_esp);
		}
		else {
			utrapframe = (struct SignalUTrapframe *)UXSTACKTOP;
		}

		utrapframe -= 1;

		user_mem_assert(curenv, 
						(const void *)(utrapframe), 
						sizeof(struct UTrapframe), 
						PTE_W | PTE_U);

		utrapframe->utf_handler = (uint32_t)(e->env_signal_handlers[signal]);
		utrapframe->utf_sysno = signal;
		utrapframe->utf_regs = e->env_tf.tf_regs;
		utrapframe->utf_eip = e->env_tf.tf_eip;
		utrapframe->utf_eflags = e->env_tf.tf_eflags;
		utrapframe->utf_esp = e->env_tf.tf_esp;

		e->env_tf.tf_eip = (uintptr_t)e->env_signal_upcall;
		e->env_tf.tf_esp = (uintptr_t)utrapframe;
	}
	else {
		if (defaultHandler[signal] == SIGNALTERMINATE) {
			signal_terminate(e, signal);
		}
		else if (defaultHandler[signal] == SIGNALIGNORE) {
			signal_ignore(e, signal);
		}
		else if (defaultHandler[signal] == SIGNALSTOP) {
			signal_stop(e, signal);
		}
		else if (defaultHandler[signal] == SIGNALCONT) {
			signal_cont(e, signal);
		}
		else {
			cprintf("signal ooops\n");
		}
	}
}