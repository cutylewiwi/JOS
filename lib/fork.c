// implement fork from user space

#include <inc/string.h>
#include <inc/lib.h>

// PTE_COW marks copy-on-write page table entries.
// It is one of the bits explicitly allocated to user processes (PTE_AVAIL).
#define PTE_COW		0x800

//
// Custom page fault handler - if faulting page is copy-on-write,
// map in our own private writable copy.
//
static void
pgfault(struct UTrapframe *utf)
{
	void *addr = (void *) utf->utf_fault_va;
	uint32_t err = utf->utf_err;
	int r;

	// Check that the faulting access was (1) a write, and (2) to a
	// copy-on-write page.  If not, panic.
	// Hint:
	//   Use the read-only page table mappings at uvpt
	//   (see <inc/memlayout.h>).

	// LAB 4: Your code here.

	if (!(err & FEC_WR)
		|| !(uvpt[PGNUM(addr)] & PTE_COW)) {
		cprintf("%p\n", addr);
		panic("pgfault(): not %s!", (err & FEC_WR) ? "COW" : "write");
	}

	// Allocate a new page, map it at a temporary location (PFTEMP),
	// copy the data from the old page to the new page, then move the new
	// page to the old page's address.
	// Hint:
	//   You should make three system calls.

	// LAB 4: Your code here.

	if ((r = sys_page_alloc(0, (void *)PFTEMP, PTE_U | PTE_W)) < 0) {
		panic("pgfault(): sys_page_alloc() failed: %e", r);
	}

	addr = (void *)(((int)addr) & ~0xFFF);
	memcpy((void *)PFTEMP, addr, PGSIZE);

	if ((r = sys_page_map(0, PFTEMP, 0, addr, PTE_U | PTE_W)) < 0) {
		panic("pgfault(): sys_page_map() failed: %e", r);
	}

	if ((r = sys_page_unmap(0, PFTEMP)) < 0) {
		panic("pgfault(): sys_page_unmap() failed: %e", r);
	}

	//panic("pgfault not implemented");
}

//
// Map our virtual page pn (address pn*PGSIZE) into the target envid
// at the same virtual address.  If the page is writable or copy-on-write,
// the new mapping must be created copy-on-write, and then our mapping must be
// marked copy-on-write as well.  (Exercise: Why do we need to mark ours
// copy-on-write again if it was already copy-on-write at the beginning of
// this function?)
//
// Returns: 0 on success, < 0 on error.
// It is also OK to panic on error.
//
static int
duppage(envid_t envid, unsigned pn)
{
	int r;

	// LAB 4: Your code here.
	//panic("duppage not implemented");

	int perm;
	void * va = (void *)(pn * PGSIZE);

	//cprintf("perm: 0x%x\n", perm);

	if (uvpt[pn] & (PTE_W | PTE_COW)) {
		perm = PTE_COW | PTE_U | PTE_P;
	}
	else {
		perm = PTE_U | PTE_P;
	}

	//asm volatile("int $3");

	if ((r = sys_page_map(0, va, envid, va, perm)) < 0) {
		panic("duppage(): sys_page_map() failed: %e\nva: %p\tenvid: %d\tperm: %08x\n", r, va, envid, perm);
	}

	if ((perm & PTE_COW) 
		&& (r = sys_page_map(0, va, 0, va, perm)) < 0) {
		panic("duppage(): sys_page_map() for self failed: %e\nva: %p\tperm: %08x\n", r, va, perm);
	}

	return 0;
}

//
// User-level fork with copy-on-write.
// Set up our page fault handler appropriately.
// Create a child.
// Copy our address space and page fault handler setup to the child.
// Then mark the child as runnable and return.
//
// Returns: child's envid to the parent, 0 to the child, < 0 on error.
// It is also OK to panic on error.
//
// Hint:
//   Use uvpd, uvpt, and duppage.
//   Remember to fix "thisenv" in the child process.
//   Neither user exception stack should ever be marked copy-on-write,
//   so you must allocate a new page for the child's user exception stack.
//
envid_t
fork(void)
{
	// LAB 4: Your code here.
	//panic("fork not implemented");

	envid_t child;
	int r;
	//int perm;
	void * va;

	set_pgfault_handler(pgfault);

	if ((child = sys_exofork()) < 0) {
		panic("fork(): sys_exofork() failed: %e", child);
	}
	else if (child == 0) {
		thisenv = &envs[ENVX(sys_getenvid())];
	}
	else {
		
		for (va = (void *)UTEXT; va < (void *)USTACKTOP; va += PGSIZE) {
			
			if (!(uvpd[PDX(va)] & PTE_P)
				|| !(uvpt[PGNUM(va)] & PTE_P)
				|| !(uvpt[PGNUM(va)] & PTE_U)) {
				continue;
			}

			duppage(child, PGNUM(va));
		}

		sys_page_alloc(child, (void *)(UXSTACKTOP - PGSIZE), PTE_U | PTE_W);

		extern void _pgfault_upcall();

		if ((r = sys_env_set_pgfault_upcall(child, _pgfault_upcall)) < 0) {
			panic("fork(): sys_env_set_pgfault_upcall() failed: %e", r);
		}

		if ((r = sys_env_set_status(child, ENV_RUNNABLE)) < 0) {
			panic("fork(): sys_env_setstatus() failed: %e", r);
		}

	}

	return child;
}

// Challenge!
int
sfork(void)
{
	panic("sfork not implemented");
	return -E_INVAL;
}
