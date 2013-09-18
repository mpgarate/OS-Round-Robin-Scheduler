
/* This must be implemented by the kernel. It will
   be called automatically during (simulated) system 
   initialization */

extern void initialize_kernel(); 


/**** Debugging 	****/
/* Note: after modifying this file remove kernel.o
	before recompiling  */

/* Set this to 1 to print out debug statements */
#define DEBUG	1

/* Macro to print debug statements and flush stdout */
#define SAY(fmt) 				SAY0(fmt)
#define SAY0(fmt) 				{if(DEBUG){printf(fmt); fflush(stdout);}}
#define SAY1(fmt,parm1) 		{if(DEBUG){printf(fmt,parm1); fflush(stdout);}}
#define SAY2(fmt,parm1,parm2) 	{if(DEBUG){printf(fmt,parm1,parm2); fflush(stdout);}}