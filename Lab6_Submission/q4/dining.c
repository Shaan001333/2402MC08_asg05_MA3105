#include "types.h"
#include "stat.h"
#include "user.h"

#define NPHIL 5
#define NCYCLES 5          // configurable cycle count

int
main(void)
{
  int pid, i, k, id;
  int left, right, first, second;

  /* forks = binary semaphores 0..4, all initially 1 (on the table) */
  for(i = 0; i < NPHIL; i++)
    sem_init(i, 1);

  /* fork() x5 -> one process per philosopher */
  id = -1;
  for(i = 0; i < NPHIL; i++){
    pid = fork();
    if(pid == 0){ id = i; break; }
  }

  if(id >= 0){
    left  = id;
    right = (id + 1) % NPHIL;

    /* DEADLOCK AVOIDANCE: resource ordering.
       Always pick the LOWER-numbered fork first, then the higher.
       (Philosopher 4 takes fork 0 first, then fork 4.) */
    first  = left < right ? left : right;
    second = left < right ? right : left;

    for(k = 1; k <= NCYCLES; k++){
      sleep(5);                       // ---- THINKING ----
      /* ---- HUNGRY: pick forks in global order ---- */
      sem_wait(first);
      sem_wait(second);
      sleep(10);                      // ---- EATING ----
      sem_signal(second);             // put forks back
      sem_signal(first);
      printf(1, "Philosopher %d: THINKING -> HUNGRY -> EATING -> THINKING (cycle %d, tick %d)\n",
             id, k, uptime());
    }
    printf(1, "Philosopher %d: FINISHED all %d cycles\n", id, NCYCLES);
    exit();
  }

  /* parent waits for all 5 philosophers */
  for(i = 0; i < NPHIL; i++)
    wait();
  printf(1, "All 5 philosophers completed %d cycles - no deadlock, no starvation.\n", NCYCLES);
  exit();
}
