# Lab 5 Question 2: Producer-Consumer (Bounded Buffer) in xv6

## Semaphore implementation (own lightweight semaphores)
- Implemented counting semaphores IN THE KERNEL (proc.c) using xv6's
  spinlock + sleep()/wakeup() primitives:
    struct ksem { struct spinlock lock; int count; };
  - sem_wait(i):  acquire lock; while(count<=0) sleep(&sem, &lock);  count--; release.
    (sleep atomically releases the lock and re-acquires it on wakeup -> no lost wakeups)
  - sem_signal(i): acquire lock; count++; wakeup(&sem); release.
- Exposed to user space via 3 new syscalls: sem_init(which,count),
  sem_wait(which), sem_signal(which)  (wired in syscall.h/.c, sysproc.c,
  defs.h, user.h, usys.S).
- Three semaphores used: empty=BUFFER_SIZE(5), full=0, mutex=1.

## Shared state approach
- Reused the Q1 shared-page syscall shm_get(): one physical page mapped at
  SHM_VA (0x60000000) into BOTH parent and child after fork() -> the circular
  buffer (buffer[5], in, out) is genuinely shared memory.
- Justification: true shared memory (not pipes) matches the classic bounded
  buffer model and lets both processes access buffer/in/out directly;
  semaphores provide synchronization, the shared page provides the data.

## Producer / Consumer behavior
- Producer (parent) is FAST (sleep 2), consumer (child) is SLOW (sleep 20),
  so the producer fills all 5 slots quickly and then BLOCKS on sem_wait(empty)
  (visible as a pause after "inserted 5"); each consumer removal signals
  empty and unblocks exactly one insertion -> ping-pong until 20 items.
- Consumer BLOCKS on sem_wait(full) whenever the buffer is empty.
- mutex (binary semaphore) protects the in/out indices during insert/remove.

## Build & run
  make clean && make qemu-nox
  $ prodcons
Expected: "Producer: inserted 1..20" and "Consumer: removed 1..20" in order,
with visible blocking pauses; final line confirms success. No item is lost,
duplicated or reordered.
