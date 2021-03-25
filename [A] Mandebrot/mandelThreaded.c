#include <pthread.h>
#include <stdio.h>
#include "mandelCore.h"
#include "mandelThreaded.h"
#include "semlib.h"
//#include <sched.h>



#define WORKING 1
#define NOT_WORKING 2
#define DONE 3

taskT *tasks;


int find_next_finished_thread(int numOfThreads)  {
  int i = 0;
  static int timesReturned = 0;
  
  while (1)  {
    if (tasks[i].status == DONE)  {
	tasks[i].status = NOT_WORKING;	
	timesReturned++;
        return i;
    }
    i = (i+1)%numOfThreads;
    //printf("%d stuck here!!!!!!!!!!\n", timesReturned);
  }

 
  return 0;
}


 
/* init_threads()
- called by the main thread
- it creates nofslices threads and initializes them as NOT_WORKING
- its thread runs forever in the waitUntiGetTask function 
*/
int init_threads(int numOfThreads)  {
  int i;

  for (i = 0; i < numOfThreads; i++)  {
    tasks[i].status = NOT_WORKING;
    if ( mybsem_init( &(tasks[i]).sem, 0) != 0 ) {
      printf("Error initializing the mutex. Exiting\n");
      return -1;
    }

    if ( pthread_create( &(tasks[i]).tid, NULL, (void *)waitUntilGetTask, (void *)&tasks[i]) != 0) {
      printf("Error creating thread %d. Exiting\n", i);
      return -1;
    }
  }

  return 0;
}


/* waitUntilGetTask()
- Called by init_threads()
- Here run all the "slave" threads, those who do the calculations
- If the main thread marks a thread as WORKING from main function
the thread calls the mandel_Calc function and then marks itself as
DONE
- If a thread is not in WORKING status, it gives the processor to
the next available thread
*/
void *waitUntilGetTask(void *newtask)  {    

  taskT *slave = (taskT *)newtask;
 // my_down( &(slave->sem) );
  while (1)  {
      down( &(slave->sem) );
      mandel_Calc( slave->pars, slave->maxIterations, slave->res );
      slave->status = DONE;
      printf("changing to done...\n");
  }

 return NULL; 
}
