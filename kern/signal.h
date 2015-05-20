/* See COPYRIGHT for copyright information. */

#ifndef JOS_KERN_SIGNAL_H
#define JOS_KERN_SIGNAL_H

#include <inc/signal.h>

int signal_kill(envid_t envid, sig_t signal);
int set_signal_handler(envid_t envid, sig_t signal, void* handler);
void signal_handle(envid_t envid, sig_t signal);

#endif /* !JOS_KERN_SIGNAL_H */