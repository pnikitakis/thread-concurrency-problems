#include <unistd.h>     /* Symbolic Constants */
#include <sys/types.h>  /* Primitive System Data Types */ 
#include <errno.h>      /* Errors */
#include <stdio.h>      /* Input/Output */
#include <stdlib.h>     /* General Utilities */
#include <pthread.h>    /* POSIX Threads */
#include <string.h>
#include "semlib.h"

#define BLUE 0
#define RED 1
#define MAXCARS 3
#define MAXWAIT 2*MAXCARS
#define SLEEPTIME 0.001*(random()%1000) /* 0.001 - 1 sec */

FILE *f; 

bsem mutSafety, 
     semRedWaiting,
     semBlueWaiting,
     semMaxCars;

volatile int flag = RED; //random start
volatile int qr = 0;
volatile int qb = 0;
volatile int numIn = 0;
volatile int maxWait = 0; //meta apo posa sunexomena amaksia 8a allaksoume pleura
volatile int qMaxCars = 0; //helping qeue gia > MAXCARS cars

void* car(void *carColor){
    
    int *clr = (int*)carColor;
    int color = *clr;
    int curCars, otherCars; //local vars
    int otherQ = 0;

    if(color == RED){
        curCars = RED;
        otherCars = BLUE;
    }
    else{
        curCars = BLUE;
        otherCars = RED;
    }
        

    down(&mutSafety);
    if(curCars == RED){
        qr++;       // eimaste neo thread sthn oura
        otherQ = qb;
    }
    else{
        qb++;
        otherQ = qr;
    }
    
    if(numIn == 0){ // kanena thread sth gefura
        flag = curCars; //thn pairnoume
        maxWait = 0;
        qMaxCars = 0;
    }
    
    if(numIn + 1 > MAXCARS){ // block ean panw apo N cars sth gefyra
        qMaxCars ++;
        up(&mutSafety);
        down(&semMaxCars);
        down(&mutSafety); 
    }
    
    if(flag == otherCars){   //an pernane oi alloi OR kairos gia allagh
        up(&mutSafety);  //eleu8eronoume mutSafety
        if(curCars == RED){
            down(&semRedWaiting);//kleidwnoume mexri an er8ei seira mas
        }else{
            down(&semBlueWaiting);
        }
        
        down(&mutSafety); //pairnoume ssafety
        if(curCars == RED){
            if(qr > 1){
                up(&semRedWaiting); //wake up next blocked thread
            }
        }
        else{
            if(qb > 1){
                up(&semBlueWaiting); //wake up next blocked thread
            }
        }
    }else if((maxWait >= MAXWAIT) && (otherQ > 0)){ //perasane panw apo 2*N cars
        if(curCars == RED){     // dwse stous allous proteraiothta
            up(&semBlueWaiting); 
            up(&mutSafety);
            down(&semRedWaiting);
        }else{
            up(&semRedWaiting); 
            up(&mutSafety);
            down(&semBlueWaiting);
        }
        down(&mutSafety); //pairnoume ssafety
        if(curCars == RED){
            if(qr > 1){
                up(&semRedWaiting); //wake up next blocked thread
            }
        }
        else{
            if(qb > 1){
                up(&semBlueWaiting); //wake up next blocked thread
            }
        }
        
    }
    
    if(curCars == RED){ //bgainoume apo oura anamonhs
        qr--;       
    }
    else{
        qb--;
    }
    numIn++;    //+1 amaksi sth gefura(emeis)
    maxWait++;  //+1 ena amaksi apo thn idia pleura(blue || red)
    
    fprintf(f, "Thread: %u\tCar: %d\n", (unsigned int)pthread_self(), curCars); 
    fflush(f);
    up(&mutSafety);
    
    
    if(curCars == BLUE){
       sleep(10);
    }
    else{
        sleep(4);
    }
    //telos gefuras. pame na bgoume   
    
    down(&mutSafety);
    numIn--;    //-1 amaksi otan feugoume
    if(qMaxCars > 0){ //an uparxei blocked car epeidh htan N cars sth gefura 
        qMaxCars--;
        up(&semMaxCars); //wake next blocked
    }
    if(numIn == 0){ //an eimastan teleutaio amaksi dwse seira stous allous
        flag = otherCars; //allagh flag
        maxWait = 0;
        qMaxCars = 0;
        if(curCars == RED){
            up(&semBlueWaiting);
        }else{
            up(&semRedWaiting);
        }
    }
    up(&mutSafety);
    
    
    
    return NULL;
}

int main(){

    f = fopen("out.txt", "w"); 
    
    int i;
    int carColor;
    pthread_t thread[50];
        
    mybsem_init(&mutSafety, 1);
    mybsem_init(&semRedWaiting, 0); 
    mybsem_init(&semBlueWaiting, 0);
    mybsem_init(&semMaxCars, 0);

    for(i = 0; i < 50; i++) {
        
        printf("For blue incoming car press [0]. For red press [1]\n");
        scanf("%d", &carColor);
        pthread_create(&thread[i], NULL, car, (void*)(&carColor));
    }
    
    printf("ready to join and finish \n\n");
    
    for(i = 0; i < 50; i++) {
        pthread_join(thread[i], NULL);
        printf("thread %d is OK\n", i);    
    }
    
    fclose(f); //trial to debug
    
    mybsem_destroy(&mutSafety);
    mybsem_destroy(&semRedWaiting);
    mybsem_destroy(&semBlueWaiting);
    mybsem_destroy(&semMaxCars);
    
    printf("Goodbye\n");
    
    return 0;
    
}