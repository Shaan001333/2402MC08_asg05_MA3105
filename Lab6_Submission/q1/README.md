# Lab 5 Question 1: Peterson's Algorithm via Shared Memory

## Design Overview
To implement Peterson's Algorithm in xv6 (where `fork()` usually creates isolated memory spaces), we introduced a lightweight **shared memory mechanism**:
1. **Shared Page Allocation:** We added a system call `int shm_get(void)` that allocates a single physical page (via `kalloc()`) and maps it into the caller's address space at a fixed virtual address (`SHM_VA = 0x60000000`).
2. **True Sharing:** By calling `shm_get()` in *both* the parent and child process *after* `fork()`, both processes obtain Page Table Entries (PTEs) pointing to the **same physical memory page**. This allows Peterson's flags (`flag[2]`, `turn`) and the shared `counter` to be genuinely shared.
3. **Safety (freevm patch):** We patched `vm.c` (specifically `deallocuvm`) to skip freeing the physical page at `SHM_VA` when a process exits, preventing a "use-after-free" bug for the surviving process.
4. **Peterson's Logic:** The user program `peterson.c` uses strict Peterson entry/exit protocols using only these shared memory variables (no OS locks), achieving mutual exclusion.

## Files Modified
- `proc.h`, `proc.c`: Added `shm_page` global and `shm_get()` implementation.
- `vm.c`: Added `shm_map()` helper and patched `deallocuvm` to protect the shared page.
- `memlayout.h`: Defined `SHM_VA` (0x60000000).
- Syscall files (`syscall.c`, `syscall.h`, `sysproc.c`, `defs.h`, `usys.S`, `user.h`): Wired the `shm_get` system call.
- `Makefile`: Added `_peterson` to `UPROGS`.

## Build & Run Instructions
1. Unzip the submission folder.
2. Run: `make clean && make qemu-nox`
3. At the xv6 shell prompt (`$`), type: `peterson`
4. **Expected Output:** You will see interleaved output from Process 0 and Process 1 incrementing a shared counter. Because Peterson's Algorithm enforces mutual exclusion, the critical section code (read-delay-write) will not race, and the final output will confirm `Final counter = 20 (expected 20)`.
