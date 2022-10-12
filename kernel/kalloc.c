// Physical memory allocator, for user processes,
// kernel stacks, page-table pages,
// and pipe buffers. Allocates whole 4096-byte pages.

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "riscv.h"
#include "defs.h"

void freerange(void *pa_start, void *pa_end);

extern char end[]; // first address after kernel.
                   // defined by kernel.ld.

struct run {
  struct run *next;
};

struct {
  struct spinlock lock;
  struct run *freelist;
   // This stores number of references to a pa
  int ref_pa[(PHYSTOP) / PGSIZE];
} kmem;

void
kinit()
{
  initlock(&kmem.lock, "kmem");
  // memset(kmem.ref_pa,1,(PHYSTOP/PGSIZE));
  freerange(end, (void*)PHYSTOP);
}

void
freerange(void *pa_start, void *pa_end)
{
  char *p;
  p = (char*)PGROUNDUP((uint64)pa_start);
  for(; p + PGSIZE <= (char*)pa_end; p += PGSIZE)
  {
    // printf("ref_pa:%d\n",kmem.ref_pa[(uint64)p/PGSIZE]);
    uint64 pa_idx=(uint64)p/PGSIZE;
    acquire(&kmem.lock);
    kmem.ref_pa[pa_idx]=1;
    release(&kmem.lock);
    kfree(p);
  }
}
int make_ref(uint64 pa)
{
  uint64 pa_idx=pa/PGSIZE,tmp;
  acquire(&kmem.lock);
  kmem.ref_pa[pa_idx]+=1;
  tmp=kmem.ref_pa[pa_idx];
  release(&kmem.lock); 
  return tmp;
}
// Get number of times a page is referred
int get_ref(uint64 pa)
{
  uint64 pa_idx=pa/PGSIZE,tmp;
  acquire(&kmem.lock);
  tmp=kmem.ref_pa[pa_idx];
  release(&kmem.lock);
  return tmp;
}

// Free the page of physical memory pointed at by pa,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void
kfree(void *pa)
{
  struct run *r;

  if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
    panic("kfree");

  uint64 pa_idx=(uint64)pa/PGSIZE;
  acquire(&kmem.lock);
  uint64 num_ref=kmem.ref_pa[pa_idx];
  release(&kmem.lock);
  // if(num_ref==0)
  // {
  //   panic("kfree(): mem freeing without reference");
  // }
  if(num_ref>1)
  {
    acquire(&kmem.lock);
    kmem.ref_pa[pa_idx]-=1;
    release(&kmem.lock);
  }
  else
  {
    // Fill with junk to catch dangling refs.
    memset(pa, 1, PGSIZE);

    r = (struct run*)pa;

    acquire(&kmem.lock);
    kmem.ref_pa[pa_idx]=0;
    r->next = kmem.freelist;
    kmem.freelist = r;
    release(&kmem.lock);
  }
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
void *
kalloc(void)
{
  struct run *r;

  acquire(&kmem.lock);
  r = kmem.freelist;
  // if(kmem.ref_pa[pa_idx]>=1)
  // {
    // panic("kalloc(): page referenced before alloc()");
  // }
  // else{
    // printf("Reached here: pa_idx=%d",pa_idx);
  // }
  uint64 pa_idx=(uint64)r/PGSIZE;
  if(r)
  {
    kmem.ref_pa[pa_idx]=1;
    kmem.freelist = r->next;
  }
  release(&kmem.lock);

  if(r)
    memset((char*)r, 5, PGSIZE); // fill with junk
  return (void*)r;
}

int cow_handler(pagetable_t pagetable,uint64 va0)
{
  uint64 va=PGROUNDDOWN(va0);
  if(va>=MAXVA)
  {
    return -1;
  }
  pte_t* pte=walk(pagetable,va,0);
  uint64 pa;
  uint flags;
  char *mem;
  if(pte==0)
  {
    return -1;
  }
  else if(((*pte)&(PTE_COW))==0)
  {
    return -2;
  }
  if((*pte&PTE_COW)!=0)
  {
    // If the page is a cow page, then
    // printf("Reached here also:pa::%d",pa);
    if((mem=kalloc())==0)
    {
      return -1;
    }
    else
    {
      pa = PTE2PA(*pte);
      // Renable write on the PTE
      *pte=(*pte)|(PTE_W);
      *pte=(*pte)&(~(PTE_COW)); // no longer a cow page
      flags = PTE_FLAGS(*pte);
      // Allocate new page to the faulted PTE if it is sharing a page
      // printf("REACHED!pa::%d::W_bit:%d\n",PTE2PA(*pte),(*pte)&PTE_W);
      // if(get_ref(pa)>1)
      // {
        memmove(mem, (char*)pa, PGSIZE);
        // *pte=(*pte)&(~PTE_V);
        // uvmunmap(pagetable,va,PGSIZE,0);
        *pte=0L;
        kfree((void*)pa);
        if(mappages(pagetable, va, PGSIZE, (uint64)mem, flags) != 0){
          kfree(mem);
          return -1;
        // goto err;
        }
      // }
      // else
        // kfree(mem);
    }
  }
  else
  {
    return -1;
  }
  return 0;
}


/*
if(pte==0)
    {
      return -1;
    }
    if(pa0 == 0)
      return -1;
    // printf("INSIDE copyout(), W_BIT::%d\n",pa0);
    if(((*pte)&(PTE_COW))!=0)  // It is a COW page
    {
      char* mem;
      if((mem=kalloc())==0)
      {
        return -1;
      }
      else
      {

        uint64 pa = PTE2PA(*pte);
        // Allocate new page to the faulted PTE if it is sharing a page
        // if(get_ref(pa)>1)
        // {
          memmove(mem, (char*)pa, PGSIZE);
          *pte=(*pte)|(PTE_W);
          *pte=(*pte)&(~(PTE_COW)); // no longer a cow page
          uint flags = PTE_FLAGS(*pte);
          // *pte=(*pte)&(~PTE_V);
          *pte=0;
          // uvmunmap(pagetable,va0,PGSIZE,0);

          kfree((void*)pa);
          
          if(mappages(pagetable, va0, PGSIZE, (uint64)mem, flags) != 0){
            kfree(mem);
            return -1;
          // goto err;
          } 
        // }
        // Renable write on the PTE

        // REMOVE PTE reference 
        // if no reference remains, free PTE
      }
*/