all:
	gcc -g -Wall -o semlib.o -c semlib.c
	gcc -g -Wall -o bridge bridge.c semlib.o -lpthread

clean:
	$(RM) bridge
	$(RM) semlib.o
	$(RM) out.txt
