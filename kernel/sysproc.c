#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"
#include "ptree.h"
#include "sysinfo.h"

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
sys_trace(void)
{
  int mask;

  argint(0, &mask);
  myproc()->tracemask = (uint)mask;
  return 0;
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
  if(n < 0)
    n = 0;
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
sys_sysinfo(void)
{
  uint64 addr;
  struct sysinfo info;
  struct proc *p = myproc();

  argaddr(0, &addr);

  // Reject buffers outside the process's user-memory size.
  if(addr >= p->sz || sizeof(info) > p->sz - addr)
    return -1;

  info.freemem = getfreemem();
  info.nproc = getnproc();

  if(copyout(p->pagetable, addr, (char *)&info,
             sizeof(info)) < 0)
    return -1;

  return 0;
}

uint64
sys_ptree(void)
{
  uint64 addr;
  int max;

  argaddr(0, &addr);
  argint(1, &max);

  if(addr == 0 || max <= 0)
    return -1;

  return ptree(addr, max);
}

// Detect user pages accessed since the previous successful call.
uint64
sys_pgaccess(void)
{
  uint64 base, mask_addr, len, va;
  int npages;
  uint mask = 0;
  pte_t *pte;
  struct proc *p = myproc();

  // Retrieve system call arguments.
  argaddr(0, &base);
  argint(1, &npages);
  argaddr(2, &mask_addr);

  // A 32-bit mask can represent at most 32 pages.
  if(npages <= 0 || npages > 32)
    return -1;

  // Require a page-aligned starting address.
  if(base % PGSIZE != 0)
    return -1;

  // Validate the entire scanned range without integer overflow.
  len = (uint64)npages * PGSIZE;
  if(base >= MAXVA || len > MAXVA - base)
    return -1;

  if(base >= p->sz || len > p->sz - base)
    return -1;

  // Validate the output buffer.
  if(mask_addr >= MAXVA || sizeof(mask) > MAXVA - mask_addr)
    return -1;

  if(mask_addr >= p->sz || sizeof(mask) > p->sz - mask_addr)
    return -1;

  // Check every page covered by the output buffer.
  for(va = PGROUNDDOWN(mask_addr);
      va <= PGROUNDDOWN(mask_addr + sizeof(mask) - 1);
      va += PGSIZE){
    pte = walk(p->pagetable, va, 0);
    if(pte == 0 || (*pte & PTE_V) == 0 ||
       (*pte & PTE_U) == 0 || (*pte & PTE_W) == 0)
      return -1;
  }

  // Validate scanned pages and collect their accessed bits.
  for(int i = 0; i < npages; i++){
    va = base + (uint64)i * PGSIZE;
    pte = walk(p->pagetable, va, 0);

    if(pte == 0 || (*pte & PTE_V) == 0 ||
       (*pte & PTE_U) == 0 || !PTE_LEAF(*pte))
      return -1;

    if(*pte & PTE_A)
      mask |= (1U << i);
  }

  // Return the bitmask to user space.
  if(copyout(p->pagetable, mask_addr,
             (char *)&mask, sizeof(mask)) < 0)
    return -1;

  // Clear accessed bits only after successful copyout.
  for(int i = 0; i < npages; i++){
    if(mask & (1U << i)){
      va = base + (uint64)i * PGSIZE;
      pte = walk(p->pagetable, va, 0);
      *pte &= ~PTE_A;
    }
  }

  return 0;
}