
typedef int BOOL;
#define TRUE 1
#define FALSE 0

/* A PID is just an int */

typedef int PID_type;

/* This is the variable (e.g. a special register) containing 
   the PID of the process that is currently running */

extern PID_type current_pid;


/* This defines the maximum number of processes that can
   be supported on this machine */
   
#define MAX_NUMBER_OF_PROCESSES 20

/* When there is no process ready to run, the current_pid
   should be set to the pid of the "idle process", 
   i.e IDLE_PROCESS (with a pid of -1). */

#define IDLE_PROCESS -1


/* Time is kept as an unsigned int, in units of 
   milliseconds */

typedef unsigned int CLOCK_TIME; /* time in ms */


/* These are the machine's registers that are used
   to store information about trap or interrupt
   that has occurred (see below). */

extern int R1, R2, R3, R4; /* registers for holding 
                              trap or interrupt info */


/* This is the clock holding the current time (in ms).
   Only the hardware can change the value of it, do
   not write to it! */

extern CLOCK_TIME clock;  /* system clock in ms */

/* A clock interrupt is generated by the hardware every 10 ms.
   Note that this may be shorter than the quantum given to
   each process. */

#define CLOCK_INTERRUPT_PERIOD 10



/* These are the interrupt numbers for various different interrupts (including
   a trap). IMPORTANT: An interrupt number corresponds to the element in the 
   INTERRUPT_TABLE containing a pointer to the interrupt handler */


/* A Trap is interrupt 0. Note the description below about the state of 
   the registers when a trap occurs, in order to determine why the trap
   occurred (disk read request, keyboard request, etc.) */

#define TRAP 0


/* The clock interrupt is interrupt 1. */

#define CLOCK_INTERRUPT 1 


/* A disk interrupt, issued by the hardware when a requested disk read
   operation has completed, is interrupt 2. The PID of process that requested the 
   disk read is placed in R1 by the hardware */

#define DISK_INTERRUPT 2   


/* A keyboard interrupt, issued by the hardware when a requested keyboard read
   operation has completed, is interrupt 2. The PID of process that requested the 
   keyboar read is placed in R1 by the hardware */

#define KEYBOARD_INTERRUPT 3  /* occurs when requested keyboard buffer is available */


/* The various kinds of TRAPs are listed below. When a trap occurs (via 
   interrupt 0, see above) register R1 will contain one of the following
   values. Register R2 will, in some of the cases as described below,
   contain a value as well. */

#define DISK_READ 0  /* Size of data should be placed in R2 */
#define DISK_WRITE 1  /* non-blocking write to disk */
#define KEYBOARD_READ 2 /* Blocking read of keyboard buffer */
#define FORK_PROGRAM 3  /* PID of new process will be in R2 */
#define END_PROGRAM 4 /* PID of ending processing will be in R2 */


/* The interrupt table, INTERRUPT_TABLE, is an array of pointers 
   to functions (with no arguments and no return type). For
   example, if the handler for a trap is the function defined by

   void handle_trap() { ... }

   then the appropriate element of the interrupt table can be assigned 
   as follows:

   INTERRUPT_TABLE[TRAP] = handle_trap;

   When a trap occurs (due to a system call by the running process), 
   the function that INTERRUPT_TABLE[TRAP] points to will automatically
   be called.  */

typedef void (*FN_TYPE)();

extern FN_TYPE INTERRUPT_TABLE[];

