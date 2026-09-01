# xv6 Process Synchronization Lab (OS Assignment 5)

This repository contains the implementations for **Operating Systems Lab Assignment 6**, focusing on classic process synchronization problems within the **xv6** teaching operating system. 

Because xv6 processes are isolated by default (e.g., `fork()` copies address spaces) and lacks built-in high-level synchronization primitives, this project involved modifying the xv6 kernel to add **custom shared memory** and **custom kernel-level counting semaphores**.

**Course:** MA3105 (Operating Systems)  
**Student:** Shaan Mujtaba  
**Roll No:** 2402MC08  

---

## 📁 Repository Structure

Each question is isolated in its own directory. Every subfolder contains the modified source files, a specific `README.md` explaining the design, `diffs/` showing exactly what was changed from the pristine xv6 kernel, and terminal output screenshots.

- **`q1-peterson/`**: Mutual Exclusion using Peterson's Algorithm.
- **`q2-prodcons/`**: Producer-Consumer Problem (Bounded Buffer).
- **`q3-readwrite/`**: Readers-Writers Problem (Fair / No Writer Starvation).

---

## 🛠️ Core Kernel Extensions

To solve these problems, two major mechanisms were added to the xv6 kernel and reused across the assignments:

### 1. Shared Memory Mechanism (`shm_get` syscall)
Since xv6's `fork()` creates completely isolated memory spaces, processes cannot share variables by default. 
* **Implementation:** Added a `int shm_get(void)` system call. It allocates a single physical page via `kalloc()` and maps it into the caller's page table at a fixed virtual address (`SHM_VA = 0x60000000`).
* **Usage:** Both parent and child processes call `shm_get()` *after* forking. This causes their Page Table Entries (PTEs) to point to the exact same physical RAM, enabling true shared memory without OS locks.
* **Safety:** Patched `vm.c` (`deallocuvm`) to skip freeing this specific physical page when a process exits, preventing use-after-free bugs.

### 2. Kernel Counting Semaphores
xv6 provides spinlocks and sleeplocks, but not counting semaphores with blocking queues.
* **Implementation:** Created a `struct ksem` in `proc.c` containing a `spinlock` and an integer `count`. 
* **Mechanism:** Used xv6's native `sleep()` and `wakeup()` primitives. If `count <= 0`, `sem_wait()` puts the process to sleep on the semaphore's address. `sem_signal()` increments the count and calls `wakeup()` to unblock waiting processes.
* **Syscalls:** Exposed to user-space via `sem_init(which, count)`, `sem_wait(which)`, and `sem_signal(which)`.

---

## 📝 Question Summaries

### Question 1: Mutual Exclusion (Peterson's Algorithm)
Implemented Peterson's Algorithm to achieve mutual exclusion between a parent and child process sharing a critical section.
* **Challenge:** Peterson's algorithm relies on shared `flag[]` and `turn` variables. Without shared memory, the algorithm silently fails because each process modifies its own private copy.
* **Solution:** Utilized the custom `shm_get()` syscall to place `flag[]`, `turn`, and a shared `counter` in the same physical page.
* **Verification:** Both processes perform a `read -> delay -> write` on the counter. The final counter reaches exactly 20, proving that mutual exclusion was strictly enforced using *only* Peterson's logic (no OS locks).

### Question 2: Producer-Consumer (Bounded Buffer)
Implemented the classic Bounded Buffer problem with a buffer size of 5, 20 items, 1 Producer, and 1 Consumer.
* **Synchronization:** Used three kernel semaphores: `empty` (count=5), `full` (count=0), and `mutex` (count=1).
* **Behavior:** The Producer is programmed to be "fast" and the Consumer "slow". The output log visibly demonstrates the Producer filling the buffer and **blocking** on the `empty` semaphore, then waking up only when the Consumer removes an item and signals.
* **Verification:** All 20 items are produced and consumed in exact order with no lost or duplicated data.

### Question 3: Readers-Writers (Fair Solution)
Implemented the Readers-Writers problem with 3 Readers and 2 Writers.
* **Challenge:** The classic "First Readers-Writers" (Reader Priority) allows an infinite stream of readers to starve a waiting writer. 
* **Solution:** Implemented the **Fair Readers-Writers** variant using a third "gatekeeper" semaphore (`readTry`). A waiting writer holds/queues on this gate, forcing *new* arriving readers to queue behind the writer instead of bypassing it.
* **Verification:** Output logs show multiple readers accessing data concurrently (same tick), writers getting exclusive access (no overlapping ticks), and writers successfully executing between reader bursts (proving no starvation).

---

## 🚀 How to Build and Run

### Prerequisites
* A Linux environment (or WSL on Windows)
* `gcc`, `make`, `qemu-system-i386`

### Running a specific question
Navigate to the specific question's folder (e.g., `q1-peterson`), clean the build, and launch xv6:

```bash
cd q1-peterson   # or q2-prodcons, or q3-readwrite
make clean
make qemu-nox
