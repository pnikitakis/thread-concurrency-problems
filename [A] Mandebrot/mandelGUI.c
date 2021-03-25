#include <stdio.h>
#include <stdlib.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include "mandelCore.h"
#include "mandelThreaded.h"
#include "semlib.h"

#define WinW 300
#define WinH 300
#define ZoomStepFactor 0.5
#define ZoomIterationFactor 2


#define WORKING 1
#define NOT_WORKING 2
#define DONE 3

static Display *dsp = NULL;
static unsigned long curC;
static Window win;
static GC gc;

extern taskT *tasks;

/* basic win management rountines */

static void openDisplay() {
  if (dsp == NULL) { 
    dsp = XOpenDisplay(NULL); 
  } 
}

static void closeDisplay() {
  if (dsp != NULL) { 
    XCloseDisplay(dsp); 
    dsp=NULL;
  }
}

void openWin(const char *title, int width, int height) {
  unsigned long blackC,whiteC;
  XSizeHints sh;
  XEvent evt;
  //long evtmsk;

  whiteC = WhitePixel(dsp, DefaultScreen(dsp));
  blackC = BlackPixel(dsp, DefaultScreen(dsp));
  curC = blackC;
 
  win = XCreateSimpleWindow(dsp, DefaultRootWindow(dsp), 0, 0, WinW, WinH, 0, blackC, whiteC);

  sh.flags=PSize|PMinSize|PMaxSize;
  sh.width=sh.min_width=sh.max_width=WinW;
  sh.height=sh.min_height=sh.max_height=WinH;
  XSetStandardProperties(dsp, win, title, title, None, NULL, 0, &sh);

  XSelectInput(dsp, win, StructureNotifyMask|KeyPressMask);
  XMapWindow(dsp, win);
  do {
    XWindowEvent(dsp, win, StructureNotifyMask, &evt);
  } while (evt.type != MapNotify);

  gc = XCreateGC(dsp, win, 0, NULL);

}

void closeWin() {
  XFreeGC(dsp, gc);
  XUnmapWindow(dsp, win);
  XDestroyWindow(dsp, win);
}

void flushDrawOps() {
  XFlush(dsp);
}

void clearWin() {
  XSetForeground(dsp, gc, WhitePixel(dsp, DefaultScreen(dsp)));
  XFillRectangle(dsp, win, gc, 0, 0, WinW, WinH);
  flushDrawOps();
  XSetForeground(dsp, gc, curC);
}

void drawPoint(int x, int y) {
  XDrawPoint(dsp, win, gc, x, WinH-y);
  flushDrawOps();
}

void getMouseCoords(int *x, int *y) {
  XEvent evt;

  XSelectInput(dsp, win, ButtonPressMask);
  do {
    XNextEvent(dsp, &evt);
  } while (evt.type != ButtonPress);
  *x=evt.xbutton.x; *y=evt.xbutton.y;
}

/* color stuff */

void setColor(char *name) {
  XColor clr1,clr2;


  if (!XAllocNamedColor(dsp, DefaultColormap(dsp, DefaultScreen(dsp)), name, &clr1, &clr2)) {
    printf("failed\n"); return;
  }
  XSetForeground(dsp, gc, clr1.pixel);
  curC = clr1.pixel;
}

char *pickColor(int v, int maxIterations) {
  static char cname[128];

  if (v == maxIterations) {
    return("black");
  }
  else {
    sprintf(cname,"rgb:%x/%x/%x",v%64,v%128,v%256);
    return(cname);
  }
}

int main(int argc, char *argv[]) {
  mandel_Pars pars,*slices;
  int i,j,x,y,nofslices,maxIterations,level,*res;
  int xoff,yoff;
  long double reEnd,imEnd,reCenter,imCenter;
  int next, done = 0;
 

  printf("\n");
  printf("This program starts by drawing the default Mandelbrot region\n");
  printf("When done, you can click with the mouse on an area of interest\n");
  printf("and the program will automatically zoom around this point\n");
  printf("\n");
  printf("Press enter to continue\n");
  getchar();

  pars.reSteps = WinW; /* never changes */
  pars.imSteps = WinH; /* never changes */
 
  /* default mandelbrot region */

  pars.reBeg = (long double) -2.0;
  reEnd = (long double) 1.0;
  pars.imBeg = (long double) -1.5;
  imEnd = (long double) 1.5;
  pars.reInc = (reEnd - pars.reBeg) / pars.reSteps;
  pars.imInc = (imEnd - pars.imBeg) / pars.imSteps;

  printf("enter max iterations (50): ");
  scanf("%d",&maxIterations);
  printf("enter no of slices: ");
  scanf("%d",&nofslices);
  
  /* adjust slices to divide win height */

  while (WinH % nofslices != 0) { nofslices++;}

  /* allocate slice parameter and result arrays */
  
  slices = (mandel_Pars *) malloc(sizeof(mandel_Pars)*nofslices);
  res = (int *) malloc(sizeof(int)*pars.reSteps*pars.imSteps);
 
  /* open window for drawing results */

  openDisplay();
  openWin(argv[0], WinW, WinH);

  level = 1;
  
  
/*************************************************************************************
 - Our code starts here
 - allocate numofslices struct tasks in order to save:
   a) the mandelCalc parameters
   b) the thread id of its task
   c) the int status of its slave-task(WORKING, NOT_WORKING, DONE)
**************************************************************************************/ 
  //allocate and initialize struct fields
  tasks = (taskT *)malloc( nofslices * sizeof(taskT) );
  if (tasks == NULL) {
    printf("Error allocating memory for tasks. Exiting\n");
    return -1;
  }

/* 
- assign jobs to each task but don't let them start
*/
  if (init_threads(nofslices) == -1) {
    return -1;
  }

 
  while (1) {
    
    clearWin();

    mandel_Slice(&pars,nofslices,slices);
    
    //y=0;
    for (i=0; i<nofslices; i++) {
     
     /*
     - mandel_Calc(&slices[i],maxIterations,&res[i*slices[i].imSteps*slices[i].reSteps]);
     - assign parameters in each thread for the above function  
     */
     (tasks)[i].pars= &slices[i];//1st
     (tasks)[i].maxIterations = maxIterations;//2nd
     (tasks)[i].res=&res[i*slices[i].imSteps*slices[i].reSteps];//3rd 
    }

    /*
    - give signal to start
    */

    for (i=0; i<nofslices; i++) {
      //(tasks)[i].status = WORKING;//signal to start
      if (up( &tasks[i].sem ) != 0)  {
        printf("error at my_up(). Exiting\n");
        return -1;
      }
    }
    
   /*
   - begin a while(1) until all parts of image are drawed
   - increment done when a slice has been drawed
   - break from while(1) when done==nofslices
   */
   done = 0; 
   while (1) {
            
      /*
      - select the first thread that finished working
      */
      printf("done is %d\n", done);      
      next = find_next_finished_thread(nofslices);
      j = next *slices[next].imSteps;//set j = the next slice begin	
      
      /*
      - when i read the border with the upper slice, exit
      - (next+1)*slices[next].imSteps is the beggining of the upper slice      
      */
      for (; j<(next+1)*slices[next].imSteps; j++) {
	for (x=0; x<slices[next].reSteps; x++) {
          setColor(pickColor(res[j*slices[next].reSteps+x],maxIterations));
          drawPoint(x,j);
        }

      }
      done++;
      if (done == nofslices)
        break;

    }

  
    //give signals "NOT_WORKING" status
    for (i=0; i<nofslices; i++) {
      tasks[i].status = NOT_WORKING;//signal to start
    } 

    /* get next focus/zoom point */
    
    getMouseCoords(&x,&y);
    xoff = x;
    yoff = WinH-y;
    
    /* adjust region and zoom factor  */
    
    reCenter = pars.reBeg + xoff*pars.reInc;
    imCenter = pars.imBeg + yoff*pars.imInc;
    pars.reInc = pars.reInc*ZoomStepFactor;
    pars.imInc = pars.imInc*ZoomStepFactor;
    pars.reBeg = reCenter - (WinW/2)*pars.reInc;
    pars.imBeg = imCenter - (WinH/2)*pars.imInc;
    
    maxIterations = maxIterations*ZoomIterationFactor;
    level++;

  } 
  
  /* never reach this point; for cosmetic reasons */
  
  free(slices);
  free(res);

  closeWin();
  closeDisplay();

}
