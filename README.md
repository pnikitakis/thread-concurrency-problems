# thread-concurrency-problems
3 different problems solved with multithreading. University project [no.2] for Concurrent Programming (Fall 2017).

## Description
### Problem A: 
Create Mandelbrot set by divinding into N parts, where each calculate has an individual painting work. Implemented by havig each part of the work assigned to a thread that return to main thread, which paints the results. 

### Problem B:
Control the traffic over a 2-way bridge, so that:
- There are no cars moving both ways
- There are no more than N cars on the brigde at any time
- There can't be any car waiting for ever
Implemented with 2 threads in each entrance of the bridge, by using semaphores.

### Problem C:
A roller coaster fits N passengers and starts only when it's full. Passengers get of when the roller coaster has finished the ride and before the new passengers come. Implemented a sychronization between passengers and roller coaster, where there is one thread for passenger and one for roller coaster.

## Prerequisites
- C
- Make (sudo apt install make)
- GCC (sudo apt install gcc)

## How to run
On terminal run `make` to build the executable C program.  
Start each problem's program with `./[program name]`.

## Authors
- [Panagiotis Nikitakis](https://www.linkedin.com/in/panagiotis-nikitakis/)
- [Christos Axelos](https://linkedin.com/in/christos-axelos-748386149)

## Course website
[ECE321 Concurrent Programming](https://www.e-ce.uth.gr/studies/undergraduate/courses/ece321/?lang=en)  
Assigment instructions can be found [here](https://github.com/pnikitakis/multithreading-intepreter/blob/main/assigment_instructions_GR.pdf) **in Greek**.
