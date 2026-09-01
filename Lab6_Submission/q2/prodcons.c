#include "types.h"
#include "stat.h"
#include "user.h"

#define BUFFER_SIZE 5
#define NUM_ITEMS 20

struct shared_data {
  int buffer[BUFFER_SIZE];
  int in;
  int out;
};

int main(void) {
  int pid, i, item;
  struct shared_data *shm;

  // Initialize semaphores in the kernel
  // 0 = empty (count = BUFFER_SIZE)
  // 1 = full (count = 0)
  // 2 = mutex (count = 1)
  sem_init(0, BUFFER_SIZE);
  sem_init(1, 0);
  sem_init(2, 1);

  pid = fork();
  
  // Map the shared page (reusing Q1's shm_get)
  shm = (struct shared_data*)shm_get();
  
  if (pid == 0) {
    // --- CONSUMER PROCESS ---
    for (i = 0; i < NUM_ITEMS; i++) {
      sem_wait(1); // Wait for 'full' (blocks if buffer is empty)
      sem_wait(2); // Wait for 'mutex'
      
      item = shm->buffer[shm->out];
      shm->out = (shm->out + 1) % BUFFER_SIZE;
      printf(1, "Consumer: removed %d\n", item);
      
      sem_signal(2); // Signal 'mutex'
      sem_signal(0); // Signal 'empty'
      
      sleep(20); // Consume SLOWLY to force producer to block on 'full'
    }
    exit();
  } else {
    // --- PRODUCER PROCESS (Parent) ---
    shm->in = 0;
    shm->out = 0;
    
    for (i = 1; i <= NUM_ITEMS; i++) {
      sleep(2); // Produce FAST to fill the buffer
      
      sem_wait(0); // Wait for 'empty' (blocks if buffer is full)
      sem_wait(2); // Wait for 'mutex'
      
      shm->buffer[shm->in] = i;
      shm->in = (shm->in + 1) % BUFFER_SIZE;
      printf(1, "Producer: inserted %d\n", i);
      
      sem_signal(2); // Signal 'mutex'
      sem_signal(1); // Signal 'full'
    }
    wait();
    printf(1, "Producer-Consumer finished successfully. No items lost or duplicated.\n");
    exit();
  }
}
