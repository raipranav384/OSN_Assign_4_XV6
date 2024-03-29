#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"

uint64
sys_exit(void)
{
  int n;
  argint(0, &n);
  exit(n);
  return 0;  // not reached
}

uint64
sys_getpid(void)
{
  return myproc()->pid;
}

uint64
sys_fork(void)
{
  return fork();
}

uint64
sys_wait(void)
{
  uint64 p;
  argaddr(0, &p);
  return wait(p);
}

uint64
sys_sbrk(void)
{
  uint64 addr;
  int n;

  argint(0, &n);
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

uint64
sys_sleep(void)
{
  int n;
  uint ticks0;

  argint(0, &n);
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(killed(myproc())){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

uint64
sys_kill(void)
{
  int pid;

  argint(0, &pid);
  return kill(pid);
}

// return how many clock tick interrupts have occurred
// since start.
uint64
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

uint64 
sys_sigalarm(void)
{
  uint64 fn;
  int n;
  argint(0,&n);
  argaddr(1,&fn);
  struct proc *p=myproc();
  // acquire(&p->lock);
  p->alarm_int=n;
  // p->handler=(void(*)())fn;
  p->handler=fn;
  p->nc_ticks=n;
  // release(&p->lock);
  return 0;
}

uint64
sys_sigreturn(void)
{
  struct proc *p=myproc();
  acquire(&p->lock);
  p->nc_ticks=p->alarm_int;
  // p->trapframe->epc=p->backup;
  // p->trapframe->epc=p->trapframe->sp;
  // p->trapframe->sp+=4;
  *(p->trapframe)=*(p->backup);
  release(&p->lock);
  return p->trapframe->a0;
}

uint64 
sys_settickets(void)
{
  struct proc *p=myproc();
  if(p==0)
    return -1;
  int tickets;
  acquire(&p->lock);
  argint(0,&tickets);
  if(tickets<0)
    return -1;
  release(&p->lock);
  p->tickets=tickets;
  return 0;
}

uint64 
sys_setpriority(void)
{
  //a0 stores arg0 -> priority
  //a1 stores arg1 -> pid

  int newPriority = 0;
  int processPID = 0;

  argint(0, &newPriority);
  argint(1, &processPID);

  return setpriority(newPriority, processPID);
}

uint64
sys_waitx(void)
{
  uint64 p, raddr, waddr;
  int rtime, wtime;
  // if(argaddr(0, &p) < 0)
  //   return -1;
  // if (argaddr(1, &raddr) < 0)
  //   return -1;
  // if (argaddr(2, &waddr) < 0)
  //   return -1;
  argaddr(0, &p) ;
  argaddr(1, &raddr); 
  argaddr(2, &waddr);

  int ret = waitx(p,&rtime,&wtime);
  struct proc *proc = myproc();
  if (copyout(proc->pagetable, raddr, (char*)&rtime , sizeof(int)) < 0)
    return -1;
  if (copyout(proc->pagetable, waddr, (char*)&wtime , sizeof(int)) < 0)
    return -1;
  return ret;
}

uint64 sys_strace(void)
{
  //mask is in a0
  //strace(arg) -> arg stored in a0, a1,  . . .a7
  // strace -> sys_strace
  int procMask;
  struct proc *p = myproc();
  argint(0, &procMask);
  // if (ret < 0)
  //   return 1;

  p->mask = procMask;

  return 0;
  //return value of sys_strace is stored in a0
}