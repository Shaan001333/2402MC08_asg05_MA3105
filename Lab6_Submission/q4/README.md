Lab 6 Question 4: Dining Philosophers - Deadlock Avoidance



STRATEGY USED: RESOURCE ORDERING (fork ordering).

Each fork is a binary kernel semaphore (custom ksem from Q2, based on xv6

sleep()/wakeup()). Philosopher i needs forks i and (i+1)%5. The protocol

forces every philosopher to pick the LOWER-numbered fork first, then the

higher one; philosopher 4 therefore takes fork 0 before fork 4, breaking

the "everyone grabs left first" symmetry.



WHY IT PREVENTS DEADLOCK:

Deadlock requires circular wait. Under this protocol a blocked philosopher

always holds his lower-numbered fork and waits only for a strictly

HIGHER-numbered fork, so fork numbers strictly increase along any

holds->waits chain. A cycle would need the numbers to increase and return

to the start, which is impossible. Hence circular wait (and deadlock)

can never form.



ALTERNATIVES CONSIDERED:

(a) Room semaphore limiting the table to 4 philosophers - also breaks

circular wait but adds an extra primitive and limits concurrency.

(b) Asymmetric odd/even picking - equivalent in spirit to ordering.

Ordering was chosen: minimal code, no extra primitives, full concurrency.



FAIRNESS / NO STARVATION:

Eat and think times are bounded (sleep 10 / sleep 5) and both forks are

always released after eating, so no philosopher is skipped indefinitely.

The log shows all 5 philosophers completing all 5 cycles and the program

terminating cleanly - a deadlocked solution would hang forever instead.



BUILD AND RUN:

make clean \&\& make qemu-nox

$ dining

