#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "riscv.h"
#include "spinlock.h"
#include "proc.h"
#include "syscall.h"
#include "defs.h"

// Fetch the uint64 at addr from the current process.
int
fetchaddr(uint64 addr, uint64 *ip)
{
  struct proc *p = myproc();
  if(addr >= p->sz || addr+sizeof(uint64) > p->sz) // both tests needed, in case of overflow
    return -1;
  if(copyin(p->pagetable, (char *)ip, addr, sizeof(*ip)) != 0)
    return -1;
  return 0;
}

// Fetch the nul-terminated string at addr from the current process.
// Returns length of string, not including nul, or -1 for error.
int
fetchstr(uint64 addr, char *buf, int max)
{
  struct proc *p = myproc();
  if(copyinstr(p->pagetable, buf, addr, max) < 0)
    return -1;
  return strlen(buf);
}

static uint64
argraw(int n)
{
  struct proc *p = myproc();
  switch (n) {
  case 0:
    return p->trapframe->a0;
  case 1:
    return p->trapframe->a1;
  case 2:
    return p->trapframe->a2;
  case 3:
    return p->trapframe->a3;
  case 4:
    return p->trapframe->a4;
  case 5:
    return p->trapframe->a5;
  }
  panic("argraw");
  return -1;
}

// Fetch the nth 32-bit system call argument.
void
argint(int n, int *ip)
{
  *ip = argraw(n);
}

// Retrieve an argument as a pointer.
// Doesn't check for legality, since
// copyin/copyout will do that.
void
argaddr(int n, uint64 *ip)
{
  *ip = argraw(n);
}

// Fetch the nth word-sized system call argument as a null-terminated string.
// Copies into buf, at most max.
// Returns string length if OK (including nul), -1 if error.
int
argstr(int n, char *buf, int max)
{
  uint64 addr;
  argaddr(n, &addr);
  return fetchstr(addr, buf, max);
}

// Prototypes for the functions that handle system calls.
extern uint64 sys_fork(void);
extern uint64 sys_exit(void);
extern uint64 sys_wait(void);
extern uint64 sys_pipe(void);
extern uint64 sys_read(void);
extern uint64 sys_kill(void);
extern uint64 sys_exec(void);
extern uint64 sys_fstat(void);
extern uint64 sys_chdir(void);
extern uint64 sys_dup(void);
extern uint64 sys_getpid(void);
extern uint64 sys_sbrk(void);
extern uint64 sys_sleep(void);
extern uint64 sys_uptime(void);
extern uint64 sys_open(void);
extern uint64 sys_write(void);
extern uint64 sys_mknod(void);
extern uint64 sys_unlink(void);
extern uint64 sys_link(void);
extern uint64 sys_mkdir(void);
extern uint64 sys_close(void);
extern uint64 sys_sigalarm(void);
extern uint64 sys_sigreturn(void);
extern uint64 sys_settickets(void);
extern uint64 sys_setpriority(void);
extern uint64 sys_waitx(void);
extern uint64 sys_strace(void);

// An array mapping syscall numbers from syscall.h
// to the function that handles the system call.
static uint64 (*syscalls[])(void) = {
[SYS_fork]      sys_fork,
[SYS_exit]      sys_exit,
[SYS_wait]      sys_wait,
[SYS_pipe]      sys_pipe,
[SYS_read]      sys_read,
[SYS_kill]      sys_kill,
[SYS_exec]      sys_exec,
[SYS_fstat]     sys_fstat,
[SYS_chdir]     sys_chdir,
[SYS_dup]       sys_dup,
[SYS_getpid]    sys_getpid,
[SYS_sbrk]      sys_sbrk,
[SYS_sleep]     sys_sleep,
[SYS_uptime]    sys_uptime,
[SYS_open]      sys_open,
[SYS_write]     sys_write,
[SYS_mknod]     sys_mknod,
[SYS_unlink]    sys_unlink,
[SYS_link]      sys_link,
[SYS_mkdir]     sys_mkdir,
[SYS_close]     sys_close,
[SYS_sigalarm]  sys_sigalarm,
[SYS_sigreturn] sys_sigreturn,
[SYS_settickets] sys_settickets,
[SYS_setpriority] sys_setpriority,
[SYS_waitx]   sys_waitx,
[SYS_strace] sys_strace

};


//Chang this to syscall.h
#define MAX_COMMANDS 27

void syscall(void)
{
  int num;
  struct proc *p = myproc();

  num = p->trapframe->a7;

  // for (int i = 0; i < 3; i++)

  // printf("%d\n", p->trapframe->a1);
  // printf("%d\n", p->trapframe->a2);

  if (num > 0 && num < NELEM(syscalls) && syscalls[num])
  {
    // Use num to lookup the system call function for num, call it,
    // and store its return value in p->trapframe->a0

    //num - index of system call ---> get system call name

    //Spec 1 - Testing

    char *names[MAX_COMMANDS + 1] = {"temp", "fork", "exit", "wait", "pipe", "read", "kill", "exec", "fstat", "chdir", "dup", "getpid", "sbrk", "sleep", "uptime", "open", "write", "mknod", "unlink", "link", "mkdir", "close", "strace", "sigalarm", "sigreturn", "settickets", "setpriority", "waitx"};
    int argCount[MAX_COMMANDS + 1] = {0, 1, 1, 1, 3, 3, 1, 2, 2, 1, 1, 1, 1, 1, 1, 2, 3, 3, 1, 2, 1, 1, 1, 2, 0, 1, 2, 3};

    //Pranav
    // if (num != 16)
    // printf("%d:: :(%d, %d, %d)\n", num, p->trapframe->a0, p->trapframe->a1, p->trapframe->a2);

    // if (num <= MAX_COMMANDS && num != 16)
    //   printf("%d:: %s :(%d, %d, %d)\n", num, names[num], p->trapframe->a0, p->trapframe->a1, p->trapframe->a2);

    // if (num <= MAX_COMMANDS && num != 16)
    //   printf("%d: syscall %s (%d, %d, %d)\n", num, names[num], p->trapframe->a0, p->trapframe->a1, p->trapframe->a2);

    //Storing values in reg a0,a1,a2 before funciton is called

    // uint64 processID = p->trapframe->a0;
    // uint64 name = p->trapframe->a1;
    // uint64 decimalValue = p->trapframe->a2;
    uint64 processListMask = p->mask;

    p->trapframe->a0 = syscalls[num]();
    uint64 returnValue = p->trapframe->a0;

    uint64 regData[4];
    regData[0] = p->trapframe->a0;
    regData[1] = p->trapframe->a1;
    regData[2] = p->trapframe->a2;

    if (processListMask & (1 << num))
    {
      //numth process is to be traced

      //LEFT: CHECK HOW MANY ARGUMENTS TO BE PRINTED
      // printf("%d: syscall %s (%d %d %d) -> %d\n", num, names[num], processID, name, decimalValue, returnValue);

      printf("%d: syscall %s (", p->pid, names[num]);
      for (int i = 0; i < argCount[num]; i++)
      {
        printf("%d", regData[i]);

        if (i != argCount[num] - 1)
          printf(" ");
      }

      printf(") -> %d\n", returnValue);
    }
    //Initial Idea

    // if (num <= MAX_COMMANDS && num != 16)
    // {
    //   // checking ith bit is set
    //   for (int i = 0; i <= MAX_COMMANDS; i++)
    //   {
    //     if (num == i && processListMask & (1 << i))
    //       printf("%d: syscall %s (%d %d %d) -> %d\n", num, names[num], processID, name, decimalValue, returnValue);
    //   }
    // }
    //a0 - registers for arg
    //a0 -> return value
  }
  else
  {
    printf("%d %s: unknown sys call %d\n",
           p->pid, p->name, num);
    p->trapframe->a0 = -1;
  }
}
