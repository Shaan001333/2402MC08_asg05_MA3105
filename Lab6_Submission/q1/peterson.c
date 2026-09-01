#include "types.h"
#include "stat.h"
#include "user.h"
#include "memlayout.h"      // for SHM_VA

struct shm_region {         // lives in the shared page
  int flag[2];
  int turn;
  int counter;
};

static volatile struct shm_region *shm;

static void delay(int n){ volatile int i; for(i = 0; i < n; i++); }

static void
enter_cs(int id)
{
  int other = 1 - id;
  shm->flag[id] = 1;        // I want to enter
  shm->turn = other;        // give priority to the other
  while(shm->flag[other] == 1 && shm->turn == other)
    ;                       // busy-wait (Peterson)
}

static void
exit_cs(int id)
{
  shm->flag[id] = 0;        // I'm out
}

int
main(void)
{
  int id, pid, i, v;

  pid = fork();
  id = (pid == 0) ? 1 : 0;          // child = 1, parent = 0

  shm = (volatile struct shm_region*)(uint)shm_get();   // AFTER fork -> shared!
  if((void*)shm == (void*)-1){
    printf(1, "shm_get failed\n");
    exit();
  }

  for(i = 0; i < 10; i++){
    enter_cs(id);
    /* critical section: read-modify-write with a race window */
    v = shm->counter;
    delay(50000);                    // would lose updates WITHOUT mutual exclusion
    shm->counter = v + 1;
    printf(1, "Process %d in CS, counter = %d\n", id, shm->counter);
    exit_cs(id);
    delay(100000);                   // remainder section
  }

  if(id == 0){
    wait();
    printf(1, "Final counter = %d (expected 20)\n", shm->counter);
  }
  exit();
}
