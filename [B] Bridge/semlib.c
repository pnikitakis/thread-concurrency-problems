#include <stdio.h>      /* Input/Output */
#include <errno.h>      /* Errors */
#include <pthread.h>    /* POSIX Threads */
#include <string.h>     /* String handling */
#include "semlib.h"

int mybsem_init(pthread_mutex_t *mutex, int value);
int down(pthread_mutex_t *mutex);
int up(pthread_mutex_t *mutex);
int mybsem_destroy(pthread_mutex_t *mutex);


int mybsem_init(pthread_mutex_t *mutex, int value){

    if(pthread_mutex_init(mutex, NULL)){
        printf("Error at mutex initialization. %s\n", strerror(errno));
        return 1;
    }
  
    if(value != 0 && value != 1){
        printf("Error. Value can take 0 or 1.\n");
        return 1;
    }
    
    if(!value)
        if(pthread_mutex_lock(mutex)){
            printf("Error at mutex init (lock). %s\n", strerror(errno));
            return 1;
        }
        
    return 0;
    
}


int mybsem_destroy(pthread_mutex_t *mutex){
    
    if(pthread_mutex_destroy(mutex) != 0){
        printf("Error at mutex destroying. %s\n Errno code %d\n", strerror(errno), errno);
        return 1;        
    } 
    
    printf("Mutex destroyed\n");

    return 0;
}

int down(pthread_mutex_t *mutex){
    
    if(pthread_mutex_lock(mutex)){
        printf("Error at mutex lock. %s\n", strerror(errno));
        return 1;
    }
    
    return 0;
}


int up(pthread_mutex_t *mutex){
    
    if(pthread_mutex_unlock(mutex)){
        printf("Error at mutex unlock. %s\n", strerror(errno));
        return 1;
    }
    
    return 0;
}

