#ifndef JOS_KERN_MONITOR_H
#define JOS_KERN_MONITOR_H
#define MAPPINGDEBUG
#ifndef JOS_KERNEL
# error "This is a JOS kernel header; user programs should not #include it"
#endif

struct Trapframe;

// Activate the kernel monitor,
// optionally providing a trap frame indicating the current state
// (NULL if none).
void monitor(struct Trapframe *tf);

// Functions implementing monitor commands.
int mon_help(int argc, char **argv, struct Trapframe *tf);
int mon_kerninfo(int argc, char **argv, struct Trapframe *tf);
int mon_backtrace(int argc, char **argv, struct Trapframe *tf);
int mon_cutytest(int argc, char **argv, struct Trapframe *tf);

#ifdef MAPPINGDEBUG
void printpage(void *, int);
void * pa2va(physaddr_t pa, int *flag);
int mon_showmappings(int argc, char **argv, struct Trapframe *tf);
int mon_chpermissons(int argc, char **argv, struct Trapframe *tf);
int mon_coredump(int argc, char **argv, struct Trapframe *tf);
int my_atoi(char * input);
#endif

// debugging for lab 3 challenge 2
int mon_continue(int argc, char **argv, struct Trapframe *tf);
int mon_singlestep(int argc, char **argv, struct Trapframe *tf);

#endif	// !JOS_KERN_MONITOR_H
