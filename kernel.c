/* 

Include this license and a link to https://github.com/mpgarate/ with all derivations. 

The MIT License (MIT)

Copyright (c) [year] [fullname]

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include <stdio.h>
#include <stdlib.h>

#include "hardware.h"
#include "drivers.h"
#include "kernel.h"


/* Set this to 1 to print out debug statements */
#define DEBUG 0

/* Macro to print debug statements and flush stdout */
#ifdef DEBUG
  #define SAY(fmt)        SAY0(fmt)
  #define SAY0(fmt)         {if(DEBUG){printf(fmt); fflush(stdout);}}
  #define SAY1(fmt,parm1)     {if(DEBUG){printf(fmt,parm1); fflush(stdout);}}
  #define SAY2(fmt,parm1,parm2)   {if(DEBUG){printf(fmt,parm1,parm2); fflush(stdout);}}
#endif
/* You may use the definitions below, if you are so inclined, to
   define the process table entries. Feel free to use your own
   definitions, though. */


typedef enum { RUNNING, READY, BLOCKED , UNINITIALIZED } PROCESS_STATE;

typedef struct process_table_entry {
  PROCESS_STATE state;
  int CPU_time_used;
  int quantum_start_time;
} PROCESS_TABLE_ENTRY;

extern PROCESS_TABLE_ENTRY process_table[];


/* A quantum is 40 ms */

#define QUANTUM 40

PROCESS_TABLE_ENTRY process_table[MAX_NUMBER_OF_PROCESSES];

/* Keep a count of active processes */
int active_processes = 0;

/* You may use this code for maintaining a queue of
   ready processes. It's simply a linked list structure
   with each element containing the PID of a ready process */    

typedef struct ready_queue_elt {
  struct ready_queue_elt *next;
  PID_type pid;
} READY_QUEUE_ELT;


READY_QUEUE_ELT *ready_queue_head = NULL;  /* head of the event queue */
READY_QUEUE_ELT *ready_queue_tail = NULL;


//places process id at end of ready queue

void queue_ready_process(PID_type pid)
{
  //  printf("QUEUEING ready PID %d\n", pid);
  READY_QUEUE_ELT *p = (READY_QUEUE_ELT *) malloc(sizeof(READY_QUEUE_ELT));
  p->pid = pid;
  p->next = NULL;
  if (ready_queue_tail == NULL) 
    if (ready_queue_head == NULL) {
      ready_queue_head = ready_queue_tail = p;
      p->next = NULL;
    }
    else {
      printf("Error: ready queue tail is NULL but ready_queue_head is not\n");
      exit(1);
    }
  else {
    ready_queue_tail->next = p;
    ready_queue_tail = p;
  }
}

//removes and returns PID from front of ready queue

PID_type dequeue_ready_process()
{
  if (ready_queue_head == NULL)
    if (ready_queue_tail == NULL)
      return IDLE_PROCESS;        // indicates no active process is ready
    else {
      printf("Error: ready_queue_head is NULL but ready_queue_tail is not\n");
      exit(1);
    }
  else {      
    READY_QUEUE_ELT *p = ready_queue_head;
    ready_queue_head = ready_queue_head->next;
    if (ready_queue_head == NULL)
      ready_queue_tail = NULL;
    return p->pid;
  }
}

/*** Print process entry and table for debugging ***/

void print_process_entry(i){
  SAY1("--- %d       ",i); // PID
  /* Print state */
  switch(process_table[i].state){
    case RUNNING:
      SAY("RNING     ");
      break;
    case READY:
      SAY("READY     ");
      break;
    case BLOCKED:
      SAY("BLOCK     ");
      break;
    case UNINITIALIZED:
      SAY("UNINI     ");
      break;
  }
  SAY1("%d                 ",process_table[i].CPU_time_used);
  SAY1("%d     ",process_table[i].quantum_start_time);
  SAY("\n");
}

void print_process_table(){
  SAY("------------------- Printing Process Table -------------------\n");
  SAY("--- PID     State     CPU_time_used     quantum_start_time ---\n");
  int i = 0;
  for(i = 0; i < MAX_NUMBER_OF_PROCESSES-1; i++){
    if (process_table[i].state != UNINITIALIZED)
    print_process_entry(i);
  }
  SAY1("--- Active processes: %d\n", active_processes);
  SAY("------------------- End Process Table Print ------------------\n");
}

/* Initialize a new process in process table */
void create_process_entry(new_process_pid){
  printf("Time %d: Creating process entry for pid %d\n",clock,new_process_pid);
  process_table[new_process_pid].state = READY;
  process_table[new_process_pid].CPU_time_used = 0; 
  process_table[new_process_pid].quantum_start_time = 0;
  active_processes++;
  queue_ready_process(new_process_pid);
}

/* Update the process table to run a process */
void run_process(pid){
  process_table[pid].state = RUNNING;
  process_table[pid].quantum_start_time = clock;
  printf("Time %d: Process %d runs\n",clock, current_pid);
}

/* Calculate and store the cpu time used */
void update_CPU_time_used(pid){
  process_table[pid].CPU_time_used += (clock - process_table[pid].quantum_start_time);
}

/* Run the next process in the queue */
void run_next_process(){
  PID_type prev_pid = current_pid;
  current_pid = dequeue_ready_process();
  if (current_pid == IDLE_PROCESS){
    if (prev_pid != IDLE_PROCESS){
      printf("Time %d: Processor is idle\n",clock);fflush(stdout);
    }
  }
  else {
    run_process(current_pid);
  }
}

/* Block the current process */
void block_current_process(){
  update_CPU_time_used(current_pid);
  process_table[current_pid].state = BLOCKED;
  process_table[current_pid].quantum_start_time = 0;
}

/* These handlers are run upon the relevant interrupt  */

void handle_disk_read() {
  SAY2("PID %d handling disk read of size: %d\n",R2,current_pid);
  disk_read_req(current_pid, R2);
  block_current_process();
  run_next_process();
}

void handle_disk_write() {
  /* This is non-blocking */
  disk_write_req(current_pid);
}

void handle_keyboard_read() {
  SAY("handling keyboad read\n");
  keyboard_read_req(current_pid);
  block_current_process();
  run_next_process();
}

void handle_fork_program() {
  SAY("handling fork program\n");
  create_process_entry(R2);
}

void handle_end_program() {
  SAY("handling end program\n");
  update_CPU_time_used(current_pid);
  printf("Time %d: Process %d exits. Total CPU time = %d\n", clock, current_pid, process_table[current_pid].CPU_time_used);
  process_table[current_pid].state = UNINITIALIZED;
  active_processes--;
  if (!active_processes) {
    printf("-- No more processes to execute --\n");
    exit(0);
  }
  else{
    run_next_process();
  }
}

void handle_trap(){
  printf("IN HANDLE_TRAP. Clock = %d, trap = %d\n",clock,R1);
  switch(R1){
    case DISK_READ:         // 0
      handle_disk_read(); 
      break;
    case DISK_WRITE:        // 1
      handle_disk_write();
      break;
    case KEYBOARD_READ:     // 2
      handle_keyboard_read();
      break;
    case FORK_PROGRAM:      // 3
      handle_fork_program();
      break;
    case END_PROGRAM:       // 4
      handle_end_program();
      break;
  }
}

void handle_clock_interrupt(){
    CLOCK_TIME quantum_time_used = clock - process_table[current_pid].quantum_start_time;
  /* Check if the current process has used up its QUANTUM */
    print_process_table();
  if(current_pid ==-1){
    run_next_process();
    return;
  }
  if (quantum_time_used >= QUANTUM){
    SAY2("PID %d has run for %d. That's enough. \n",current_pid, quantum_time_used);
    update_CPU_time_used(current_pid);
    process_table[current_pid].state = READY;
    process_table[current_pid].quantum_start_time = 0;
    queue_ready_process(current_pid);
    run_next_process();
  }
}

/* Set the interrupting process to READY and queue it */
void update_interrupting_process(){
  process_table[R1].state = READY;
  process_table[R1].quantum_start_time = 0;
  queue_ready_process(R1);

  /* If the CPU is idle, we can run this interrupting
  process right away. Remove this if statement and
  block to match ben_system timing */
  if (current_pid == -1) { 
    run_next_process(); 
  }
}

void handle_disk_interrupt(){
  printf("Time %d: Handled DISK_INTERRUPT for pid %d\n", clock, R1); fflush(stdout);
  update_interrupting_process();
}

void handle_keyboard_interrupt(){
  printf("Time %d: Handled KEYBOARD_INTERRUPT for pid %d\n", clock, R1); fflush(stdout);
  update_interrupting_process();
}

/* This procedure is automatically called when the 
   (simulated) machine boots up */

void initialize_kernel()
{

  /* Add the initial process to the table */
  process_table[0].state = RUNNING;
  process_table[0].CPU_time_used = 0;
  process_table[0].quantum_start_time = clock;
  active_processes++;

  /* initialize the process table */
  int i = 1;
  for(i = 1; i < MAX_NUMBER_OF_PROCESSES-1; i++){
    process_table[i].state = UNINITIALIZED;
  }

  INTERRUPT_TABLE[TRAP] = handle_trap;
  INTERRUPT_TABLE[CLOCK_INTERRUPT] = handle_clock_interrupt;
  INTERRUPT_TABLE[DISK_INTERRUPT] = handle_disk_interrupt;
  INTERRUPT_TABLE[KEYBOARD_INTERRUPT] = handle_keyboard_interrupt;

}

