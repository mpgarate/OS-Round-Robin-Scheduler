#include <stdio.h>
#include <stdlib.h>

#include "hardware.h"
#include "drivers.h"
#include "kernel.h"


/* Set this to 1 to print out debug statements */
#define DEBUG 1

/* Macro to print debug statements and flush stdout */
#define SAY(fmt)        SAY0(fmt)
#define SAY0(fmt)         {if(DEBUG){printf(fmt); fflush(stdout);}}
#define SAY1(fmt,parm1)     {if(DEBUG){printf(fmt,parm1); fflush(stdout);}}
#define SAY2(fmt,parm1,parm2)   {if(DEBUG){printf(fmt,parm1,parm2); fflush(stdout);}}

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

/* */
void create_process_entry(){
  SAY2("Time %d: Creating process entry for pid %d\n",clock,R2);
  PROCESS_TABLE_ENTRY new_process;
  new_process.state = READY;
  new_process.CPU_time_used = 0; 
  new_process.quantum_start_time = 0;
  process_table[current_pid] = new_process;
  queue_ready_process(current_pid);
}

/* These handlers are run upon the relevant interrupt  */

void handle_disk_read() {
  SAY("handling disk read\n");
  disk_read_req(R1, R2);
  dequeue_ready_process();
}
void handle_disk_write() {
  /* This is non-blocking */
  /* Process continues while data is being 
     written to the disk */
  SAY("handling disk write\n");
}
void handle_keyboard_read() {
  SAY("handling keyboad read\n");
  keyboard_read_req(current_pid);
}
void handle_fork_program() {
  SAY("handling fork program\n");
  if (fork(R2) == 0){
    /* Child Process */
    current_pid += 1;
    create_process_entry();
    queue_ready_process(current_pid);
  }
}
void handle_end_program() {
  SAY("handling end program\n");
  SAY2("Process %d exits. Total CPU time = %d\n", current_pid, process_table[current_pid].CPU_time_used);
  exit(0);
}

void handle_trap(){
  SAY2("In handle_trap. Clock: %d trap: %d \n",clock,R1);
  switch(R1){
    case DISK_READ:
      handle_disk_read(); 
      break;
    case DISK_WRITE:
      handle_disk_write();
      break;
    case KEYBOARD_READ:
      handle_keyboard_read();
      break;
    case FORK_PROGRAM:
      handle_fork_program();
      break;
    case END_PROGRAM:
      handle_end_program();
      break;
  }
}

void handle_clock_interrupt(){
  SAY("Handling clock interrupt\n");
  PROCESS_TABLE_ENTRY current_process = process_table[current_pid];
  current_process.CPU_time_used = clock - current_process.quantum_start_time*QUANTUM;
  SAY2("PID %d has used %d time\n",current_pid,current_process.CPU_time_used);
}

void handle_disk_interrupt(){
  SAY("Handling disk interrupt\n");
}

void handle_keyboard_interrupt(){
  SAY("Handling keyboard interrupt\n");
}
/* This procedure is automatically called when the 
   (simulated) machine boots up */

void initialize_kernel()
{
  SAY("Initializing Kernel\n");
  // Put any initialization code you want here.
  // Remember, the process 0 will automatically be
  // executed after initialization (and current_pid
  // will automatically be set to 0), 
  // so the your process table should reflect that fact.

  /* Add the initial process to the table */
  create_process_entry();

  INTERRUPT_TABLE[TRAP] = handle_trap;
  INTERRUPT_TABLE[CLOCK_INTERRUPT] = handle_clock_interrupt;
  INTERRUPT_TABLE[DISK_INTERRUPT] = handle_disk_interrupt;
  INTERRUPT_TABLE[KEYBOARD_INTERRUPT] = handle_keyboard_interrupt;

}

