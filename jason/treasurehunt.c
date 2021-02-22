#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/errno.h>
#include <assert.h>
#include "uthread.h"
#include "queue.h"
#include "disk.h"

queue_t pending_read_queue;
int val;
int result;
int counter;

void interrupt_service_routine() {
  // TODO
  void* result;
  void* counter;
  void (*callback)(void*,void*);
  queue_dequeue(pending_read_queue, &result, &counter, &callback);
  callback(result, counter);
}

void handleOtherReads (void* resultv, void* countv) {
  // TODO
  int *cur_count = countv;
  (*cur_count)--;
  int *result = resultv;
  val = *result;
  if (*cur_count <= 0) {
    printf("%d\n", val);
    exit(EXIT_SUCCESS);
  } else {
    queue_enqueue(pending_read_queue, result, cur_count, handleOtherReads);
    disk_schedule_read(result, *result);
  }
}

void handleFirstRead (void* resultv, void* countv) {
  // TODO
  //read first block
  int *result = resultv;
  int *counter = countv;
  //value is how many more blocks we need to read and the value of the block we start reading at 
  *counter = *result;
  if (*counter != 0) {
    queue_enqueue(pending_read_queue, result, counter, handleOtherReads);
    disk_schedule_read(result, *result);
  } else {
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
  queue_enqueue(pending_read_queue, &result, &counter, handleFirstRead);
  disk_schedule_read(&result, starting_block_number);


  while (1); // inifinite loop so that main doesn't return before hunt completes
}