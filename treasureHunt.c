#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/errno.h>
#include <assert.h>
#include "uthread.h"
#include "queue.h"
#include "disk.h"

queue_t pending_read_queue;
int treasure;
int counter;

void interrupt_service_routine() {
  // TODO
  void* val;
  void* arg;
  void (*callback)(void*,void*);
  queue_dequeue (pending_read_queue, &val, &arg, &callback);
  callback (val, arg);
}

void handleOtherReads (void* resultv, void* countv) {
  // TODO
  int * result = resultv;
  int * count = countv;
  *count = *count - 1;
  //printf("Value of block: %d\n", *result);
  //printf("Counter: %d\n", *count);
  if(*count != 0){
      queue_enqueue(pending_read_queue, result, count, handleOtherReads);
      disk_schedule_read(&treasure, *result);
  } else {
      printf("%d\n", treasure);
      exit(EXIT_SUCCESS);
  }
}

void handleFirstRead (void* resultv, void* countv) {
  // TODO
  int * result = resultv;
  int * count = countv;
  *count = treasure;
  //printf("Value of block: %d\n", *result);
  //printf("counter: %d\n", *count);
  if (*count != 0) {
    queue_enqueue(pending_read_queue, result, count, handleOtherReads);
    disk_schedule_read(&treasure, *count);
  } else {
    printf("%d\n", treasure);
    exit(EXIT_SUCCESS);
  }
}

int main (int argc, char** argv) {
  // Command Line Arguments
  static char* usage = "usage: treasureHunt starting_block_number";
  int starting_block_number;
  char *endptr;
  if (argc == 2)
    starting_block_number = strtol (argv [1], &endptr, 10);
  if (argc != 2 || *endptr != 0) {
    printf ("argument error - %s \n", usage);
    return EXIT_FAILURE;
  }

  // Initialize
  uthread_init (1);
  disk_start (interrupt_service_routine);
  pending_read_queue = queue_create();


  // Start the Hunt
  // TODO
  //printf("The first block: %d\n", starting_block_number);

  queue_enqueue(pending_read_queue, &treasure, &counter, handleFirstRead);
  disk_schedule_read(&treasure, starting_block_number);

  while (1); // inifinite loop so that main doesn't return before hunt completes
}
