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
    kmem.ref_pa[pa_idx]=1;
    kfree(p);
  }
}
int make_ref(uint64 pa)
{
  uint64 pa_idx=pa/PGSIZE;
  acquire(&kmem.lock);
  kmem.ref_pa[pa_idx]+=1;
  release(&kmem.lock); 
  return kmem.ref_pa[pa_idx];
}
// Get number of times a page is referred
int get_ref(uint64 pa)
{
  uint64 pa_idx=pa/PGSIZE;
  return kmem.ref_pa[pa_idx];
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
  if(num_ref==0)
  {
    panic("kfree(): mem freeing without reference");
  }
  if(num_ref>1)
  {
    acquire(&kmem.lock);
    kmem.ref_pa[pa_idx]-=1;
    release(&kmem.lock);
  }
  else
  {
    kmem.ref_pa[pa_idx]=0;
    // Fill with junk to catch dangling refs.
    memset(pa, 1, PGSIZE);

    r = (struct run*)pa;

    acquire(&kmem.lock);
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
  uint64 pa_idx=(uint64)r/PGSIZE;
  if(kmem.ref_pa[pa_idx]>=1)
  {
    panic("kalloc(): page referenced before alloc()");
  }
  else{
    // printf("Reached here: pa_idx=%d",pa_idx);
    kmem.ref_pa[pa_idx]+=1;
  }
  if(r)
    kmem.freelist = r->next;
  release(&kmem.lock);

  if(r)
    memset((char*)r, 5, PGSIZE); // fill with junk
  return (void*)r;
}
