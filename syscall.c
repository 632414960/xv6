#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "x86.h"
#include "syscall.h"

extern unsigned long numcalls;

// User code makes a system call with INT T_SYSCALL.
// System call number in %eax.
// Arguments on the stack, from the user call to the C
// library system call function. The saved user %esp points
// to a saved program counter, and then the first argument.

// Fetch the int at addr from the current process.
int
fetchint(uint addr, int *ip)
{
  if(addr >= proc->sz || addr+4 > proc->sz)
    return -1;
  *ip = *(int*)(addr);
  return 0;
}

// Fetch the nul-terminated string at addr from the current process.
// Doesn't actually copy the string - just sets *pp to point at it.
// Returns length of string, not including nul.
int
fetchstr(uint addr, char **pp)
{
  char *s, *ep;

  if(addr >= proc->sz)
    return -1;
  *pp = (char*)addr;
  ep = (char*)proc->sz;
  for(s = *pp; s < ep; s++)
    if(*s == 0)
      return s - *pp;
  return -1;
}

// Fetch the nth 32-bit system call argument.
int
argint(int n, int *ip)
{
  return fetchint(proc->tf->esp + 4 + 4*n, ip);
}

// Fetch the nth word-sized system call argument as a pointer
// to a block of memory of size n bytes.  Check that the pointer
// lies within the process address space.
int
argptr(int n, char **pp, int size)
{
  int i;
  
  if(argint(n, &i) < 0)
    return -1;
  if((uint)i >= proc->sz || (uint)i+size > proc->sz)
    return -1;
  *pp = (char*)i;
  return 0;
}

// Fetch the nth word-sized system call argument as a string pointer.
// Check that the pointer is valid and the string is nul-terminated.
// (There is no shared writable memory, so the string can't change
// between this check and being used by the kernel.)
int
argstr(int n, char **pp)
{
  int addr;
  if(argint(n, &addr) < 0)
    return -1;
  return fetchstr(addr, pp);
}

extern int sys_chdir(void);
extern int sys_close(void);
extern int sys_dup(void);
extern int sys_exec(void);
extern int sys_exit(void);
extern int sys_fork(void);
extern int sys_fstat(void);
extern int sys_getpid(void);
extern int sys_kill(void);
extern int sys_link(void);
extern int sys_mkdir(void);
extern int sys_mknod(void);
extern int sys_open(void);
extern int sys_pipe(void);
extern int sys_read(void);
extern int sys_sbrk(void);
extern int sys_sleep(void);
extern int sys_unlink(void);
extern int sys_wait(void);
extern int sys_write(void);
extern int sys_uptime(void);
extern int sys_waitpid(void);
extern int sys_setpriority(void);
extern int sys_getpriority(void);
extern int sys_count(void);
extern int sys_exitinfo(void);
extern int sys_schedinfoinit(void);
extern int sys_schedinfo(void);

extern int sys_clone(void);
extern int sys_thread_exit(void);

extern int sys_sem_init(void);
extern int sys_sem_wait(void);
extern int sys_sem_signal(void);
extern int sys_sem_broadcast(void);

extern int sys_test_init(void);
extern int sys_update_num(void);
extern int sys_get_num(void);

static int (*syscalls[])(void) = {
[SYS_fork]          sys_fork,
[SYS_exit]          sys_exit,
[SYS_wait]          sys_wait,
[SYS_pipe]          sys_pipe,
[SYS_read]          sys_read,
[SYS_kill]          sys_kill,
[SYS_exec]          sys_exec,
[SYS_fstat]         sys_fstat,
[SYS_chdir]         sys_chdir,
[SYS_dup]           sys_dup,
[SYS_getpid]        sys_getpid,
[SYS_sbrk]          sys_sbrk,
[SYS_sleep]         sys_sleep,
[SYS_uptime]        sys_uptime,
[SYS_open]          sys_open,
[SYS_write]         sys_write,
[SYS_mknod]         sys_mknod,
[SYS_unlink]        sys_unlink,
[SYS_link]          sys_link,
[SYS_mkdir]         sys_mkdir,
[SYS_close]         sys_close,
[SYS_waitpid]       sys_waitpid,
[SYS_setpriority]   sys_setpriority,
[SYS_getpriority]   sys_getpriority,
[SYS_count]         sys_count,
[SYS_exitinfo]      sys_exitinfo,
[SYS_schedinfoinit] sys_schedinfoinit,
[SYS_schedinfo]     sys_schedinfo,
[SYS_test_init]     sys_test_init,
[SYS_clone]         sys_clone,
[SYS_thread_exit]   sys_thread_exit,
[SYS_sem_init]      sys_sem_init,
[SYS_sem_wait]      sys_sem_wait,
[SYS_sem_signal]    sys_sem_signal,
[SYS_sem_broadcast] sys_sem_broadcast,
[SYS_update_num]    sys_update_num,
[SYS_get_num]       sys_get_num,
};

void
syscall(void)
{
  int num;

  num = proc->tf->eax;
  if(num > 0 && num < NELEM(syscalls) && syscalls[num]) {
    proc->tf->eax = syscalls[num]();
    numcalls++;
  } else {
    cprintf("%d %s: unknown sys call %d\n",
            proc->pid, proc->name, num);
    proc->tf->eax = -1;
  }
}
