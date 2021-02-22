#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <sys/errno.h>
#include <assert.h>
#include "queue.h"
#include "disk.h"
#include "uthread.h"

queue_t      pending_read_queue;
unsigned int sum = 0;

void interrupt_service_routine () {
  // TODO
  void* val;
  void* arg;
  void (*callback)(void*,void*);

  queue_dequeue (pending_read_queue, &val, &arg, &callback);
  uthread_unblock(val);
}

void* read_block (void* blocknov) {
  // TODO schedule read and the update (correctly)
  uthread_t self = uthread_self();
  int result;
  int * blocknum = blocknov;

  queue_enqueue (pending_read_queue, self, NULL, NULL);
  disk_schedule_read(&result, *blocknum);
  uthread_block(self); 
  
  sum = sum + result;
  return NULL;
}

int main (int argc, char** argv) {

  // Command Line Arguments
  static char* usage = "usage: tRead num_blocks";
  int num_blocks;
  char *endptr;
  if (argc == 2)
    num_blocks = strtol (argv [1], &endptr, 10);
  if (argc != 2 || *endptr != 0) {
    printf ("argument error - %s \n", usage);
    return EXIT_FAILURE;
  }

  // Initialize
  uthread_init (1);
  disk_start (interrupt_service_routine);
  pending_read_queue = queue_create();

  uthread_t blockthread[num_blocks];
  int returnid[num_blocks];

  for(int b = 0; b < num_blocks; b++) {
    returnid[b] = b;
    blockthread[b] = uthread_create(read_block, &returnid[b]);
  }
  
  for(int b = 0; b < num_blocks; b++) {
    uthread_join(blockthread[b], NULL);
  }
  
  
  printf("%d\n", sum);
  
  exit (EXIT_SUCCESS);
}

