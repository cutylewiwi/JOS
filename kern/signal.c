

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

int 
kill(envid_t envid, sig_t signal)
{
	struct Env * e;
	int r;

	cprintf("kill\n");

	if ((r = envid2env(envid, &e, 0)) < 0) {
		return r;
	}

	e->env_signal_vector |= (1 << signal);

	cprintf("kill\n");

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
signal_abort(struct Env * env, sig_t signal)
{
	cprintf("env [%08x] aborted by signal %d\n", env->env_id, signal);
	//panic("sigabort");
	env_destroy(env);
	return 0;
}

void
signal_handle(envid_t envid, sig_t signal)
{
	struct Env * e;
	int r;
	void * handler;
	struct SignalUTrapframe * utrapframe;

	cprintf("handle\n");

	if ((r = envid2env(envid, &e, 0)) < 0) {
		//return r;
		panic("signal_handle: failed: %e", r);
	}

	e->env_signal_vector &= ~((unsigned)1<<signal);
	cprintf("vector: %08x\n", e->env_signal_vector);

	if (e->env_signal_handlers[signal]) {
		cprintf("2\n");
		cprintf("envid: %08x\n", e->env_id);
		if (e->env_tf.tf_esp >= UXSTACKTOP - PGSIZE
			&& e->env_tf.tf_esp < UXSTACKTOP) {
			utrapframe = (struct SignalUTrapframe *)(e->env_tf.tf_esp);
		}
		else {
			utrapframe = (struct SignalUTrapframe *)UXSTACKTOP;
		}

		cprintf("3\n");

		utrapframe -= sizeof(struct SignalUTrapframe);

		user_mem_assert(curenv, 
				(const void *)(utrapframe), 
				sizeof(struct UTrapframe), 
				PTE_W | PTE_U);

		cprintf("first: %d\n", *(int *)utrapframe);

		cprintf("%08x\n", &utrapframe->utf_handler);
		utrapframe->utf_handler = (uint32_t)(e->env_signal_handlers[signal]);
		cprintf("handler: %p\n", (void *)utrapframe->utf_handler);
		cprintf("4\n");
		utrapframe->utf_sysno = signal;
		utrapframe->utf_regs = e->env_tf.tf_regs;
		utrapframe->utf_eip = e->env_tf.tf_eip;
		utrapframe->utf_eflags = e->env_tf.tf_eflags;
		utrapframe->utf_esp = e->env_tf.tf_esp;

		cprintf("signo: %d\n", signal);
		cprintf("%p\n", utrapframe->utf_eip);
		cprintf("%p\n", utrapframe->utf_esp);

		e->env_tf.tf_eip = (uintptr_t)e->env_signal_upcall;
		cprintf("%08x\n", e->env_tf.tf_eip);
		cprintf("111\n");
	}
	else {
		cprintf("no handler\n");
		//signal_abort(e, signal);
	}
}