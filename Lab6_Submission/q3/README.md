# Lab 6 Question 3: Readers-Writers (Fair / no-writer-starvation solution)

## Why NOT the plain "first readers-writers"
Classic readers-priority lets an endless stream of readers keep `read_count > 0`,
so a waiting writer can be postponed forever (writer starvation).
The assignment requires no indefinite writer starvation, so I implemented the
FAIR variant with a third semaphore `readTry` as a gatekeeper.

## Semaphores (own kernel semaphores from Q2: spinlock + sleep/wakeup)
- mutex (0): protects the shared `read_count` integer.
- wrt   (1): mutual exclusion over `shared_data` (writers exclusive; first
  reader takes it, last reader releases it).
- readTry (2): fairness gate. A waiting writer holds/queues on it, so NEW
  readers queue BEHIND the writer instead of jumping ahead -> the writer is
  guaranteed to run once active readers finish.

## Logic
Writer : wait(readTry); wait(wrt);  WRITE;  signal(wrt);  signal(readTry)
Reader : wait(readTry); wait(mutex); read_count++;
         if(read_count==1) wait(wrt); signal(mutex); signal(readTry);
         READ (concurrently with other readers);
         wait(mutex); read_count--;
         if(read_count==0) signal(wrt); signal(mutex)

## Shared state
`shared_data` and `read_count` live in the shared page from Q1's shm_get()
(one physical page mapped into all 5 processes after fork()).

## Processes
fork() spawns 3 readers + 2 writers (parent waits). Each loops 3 times.

## Build & run
  make clean && make qemu-nox
  $ readwrite
Expected: several READER lines at the same tick with active readers > 1
(concurrent reads); WRITER sections isolated by its 50-tick hold (exclusive);
writers appear between reader bursts (not starved); final value = 6.
