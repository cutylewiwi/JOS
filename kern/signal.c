#include <kern/signal.h>
#include <kern/env.h>

int 
kill(envid_t envid, sig_t signal)
{
	struct Env * e;
	int r;

	if ((r = envid2env(envid, &e, 0)) < 0) {
		return r;
	}

	e->signal_vector |= (1 << signal);

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

	e->signal_handlers[signal] = handler;

	return 0;
}

void 
signal_handle(envid_t envid, sig_t signal)
{
	struct Env * e;
	int r;
	void * handler;

	if ((r = envid2env(envid, &e, 0)) < 0) {
		//return r;
		return;
	}

	//e->env_tf.tf_eip = e->signal_handlers[signal];
}

static int 
signal_abort()
{
	return 0;
}