all:
	gcc -g -Wall -o semlib.o -c semlib.c
	gcc -g -Wall -o simulation simulation.c semlib.o -lpthread

clean:
	$(RM) simulation
	$(RM) semlib.o
