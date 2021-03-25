#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>

#include "semlib.h"



struct passenger {
  pthread_t tid;
  int passId;
  int arrivalTime;
};
typedef struct passenger passengerT;


volatile int curr_time=0;
volatile int passInsideTrain = 0;
volatile int trainSize;

bsem allowEntrance, waitForEntrance;
bsem allowExit, waitForExit;
bsem beginTrain;


void *thread_passCode(void *passArgs)  {

  passengerT *args = (passengerT *)passArgs;
  
  printf("Passenger: passenger %d waiting for entrance...\n", args->passId);// , args->arrivalTime, curr_time);
  down( &allowEntrance );
  passInsideTrain++;
  printf("Passenger: passenger %d got inside the train. Total passengers: %d. Arrival time: %d secs, Exit time: %d secs\n",args->passId, passInsideTrain, args->arrivalTime, curr_time);
  up( &waitForEntrance );
  
  if (passInsideTrain == trainSize)  {
    printf("Passenger: I am number %d, the last passenger. Start the train\n", args->passId);
    
    up( &beginTrain );
  }  
    
 /*
  * Now the train is traveling...
  */ 
   
  //printf("Passenger: passenger %d waiting for exit...\n", args->passId );
  down( &allowExit );
  passInsideTrain--;
  printf("Passenger: passenger %d exited the train. Total passengers: %d. Arrival time: %d secs, Exit time: %d secs\n", args->passId, passInsideTrain, args->arrivalTime, curr_time);fflush(stdout);
  
  up( &waitForExit);  


  free( args );  

    
  return NULL;
}

int main( int argc, char *argv[] )  {
  passengerT *passArgs;
  int dailyPassengers = 0, waitingPassengers = 0;
  
  int fd;//control file descriptor
  int i;
  char buffer[3];
  int numOfRoutes = 1;
  int eof = 0;
  
  /*
   *open file including the arrival times
   */  
  fd = open(argv[1], O_RDONLY, S_IRWXU);
  if (fd < 0) {
    printf("Error opening control file. Exiting\n");
    return -1;
  }
  
  
  /*
   * Initialize the semaphores
   */
  if ( mybsem_init( &allowEntrance, 0) == -1)  {
    printf("Error initializing \"allowEntrance\" semaphore. Exiting\n");
    return -1;
  }
  if ( mybsem_init( &waitForEntrance, 0) == -1)  {
    printf("Error initializing \"allowEntrance\" semaphore. Exiting\n");
    return -1;
  }
  if ( mybsem_init( &allowExit, 0) == -1)  {
    printf("Error initializing \"allowEntrance\" semaphore. Exiting\n");
    return -1;
  }
  if ( mybsem_init( &waitForExit, 0) == -1)  {
    printf("Error initializing \"allowEntrance\" semaphore. Exiting\n");
    return -1;
  }
  if ( mybsem_init( &beginTrain, 0) == -1)  {
    printf("Error initializing \"allowEntrance\" semaphore. Exiting\n");
    return -1;
  }

  printf("Give the size of the train: ");
  scanf("%d", &trainSize);
  
  
  printf("\n**************** Start the Process ******************************\n\n");
  
  
 /*
  * find the nexts(passengers) from the input file, according to their arrival time and put the to the train
  */
  
  while (1) {
   printf("\n**************** Next route: %d ************************\n", numOfRoutes); 
   /*
    * begin the next route
    */
    while( (/*waitingPassengers < trainSize && */curr_time <= atoi(buffer) ) &&  eof == 0 )  {//twra 3s...irthe sta 4
 	//printf("reading: %d, current: %d\n", atoi(buffer), curr_time); 
      i = -1;
      do {
        i++;
        if (0 == read(fd, buffer+i, sizeof(char) ) ) { //EOF reached
	  eof = 1;	      
        }  
      
      } while (buffer[i] != '\n' && eof != 1);
     

      buffer[i] = '\0';	
      if ( atoi(buffer) > curr_time ) { // havent arived yet
        lseek(fd, -( strlen(buffer)+1 ), SEEK_CUR );
        curr_time++;
	if (waitingPassengers >= trainSize)
	  break;
      }
      else { 
        dailyPassengers++;
        waitingPassengers++;
 
        passArgs = (passengerT *)malloc( sizeof(passengerT) );
        passArgs->passId=dailyPassengers;
        passArgs->arrivalTime = atoi(buffer);
        
  
        pthread_create( &(passArgs)->tid, NULL, (void *)thread_passCode, (void *)passArgs );
        //printf("Waiting Passengers: %d, current time: %dsecs, arrival time of current passenger: %dsecs. A thread created for him\n", waitingPassengers, curr_time, atoi(buffer) );
      }


      if (eof == 1)
        break;     	

    } 
  
   printf("\nTrain Officer: Number of passengers waiting: %d. Number of passengers on board: %d\n\n", waitingPassengers, passInsideTrain);
    waitingPassengers = waitingPassengers - trainSize;
       /*
        * Now train code to insert the next passenger into the train
        */
    while (1)  {
        
	if (passInsideTrain == trainSize)
          break;
        else {
	  printf("Train: Train ready to allow entrance to passenger\n");
	  up( &allowEntrance );
	 
          down( &waitForEntrance );        
	}
        //printf("Train: Train received the signal from passenger to get in...\n");
      
    }
  
    printf("\n**************************************\nTrain: waiting for train to begin\n");
    down( &beginTrain );
    printf("Train: I received the signal from the last passenger to start\n");
 
  
   /*
    * train beginned
    */
    printf("Train: I am full and I start traveling\n");

   /*
    * train stopped
    */
 
 
   printf("Train: I reached the destination\n**************************************\n\n"); 
 
  
   /*
    * Now put these passengers out of the train
    */
    while (passInsideTrain != 0)  {
      printf("Train: Train allowed exit\n");fflush(stdout);
      up( &allowExit );
    
      down( &waitForExit );
      printf("Train: received the signal from passenger that exited...\n");fflush(stdout);
    }
  
   if (waitingPassengers == 0 && eof == 1)
     break;
 
   numOfRoutes++;

 } 
  
  printf("************** End Of Process ******************************\n\n");


  if (mybsem_destroy( &allowEntrance ) == -1)
    return -1;

  if (mybsem_destroy( &waitForEntrance ) == -1)
    return -1;

  if (mybsem_destroy( &allowExit ) == -1)
    return -1;

  if (mybsem_destroy( &waitForExit ) == -1)
    return -1;
  
  if (mybsem_destroy( &beginTrain ) == -1)
    return -1;
 

  return 0;
}
