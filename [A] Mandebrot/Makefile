mandelGUI: mandelGUI.o mandelCore.o mandelThreaded.o semlib.o
	gcc  $(CSWITCHES)  -Wall mandelGUI.o mandelCore.o mandelThreaded.o semlib.o -o mandelGUI -lpthread -lX11
	
mandelGUI.o: mandelGUI.c mandelCore.h mandelThreaded.h  semlib.h
	gcc  $(CSWITCHES) -Wall -c mandelGUI.c 

mandelCore.o: mandelCore.c mandelCore.h
	gcc -Wall -c mandelCore.c 

mandelThreaded.o: mandelThreaded.c mandelThreaded.h
	gcc -Wall -c mandelThreaded.c -lpthread	

semlib.o: semlib.c semlib.h
	gcc -g -Wall -o semlib.o -c semlib.c

clean:
	$(RM) *.o
	$(RM) mandelGUI
