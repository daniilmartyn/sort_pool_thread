CC = gcc
COPTS =-g -Wall -std=c99 -lpthread -U__STRICT_ANSI__
JUNK = *.o *~ *.gch *.dSYM sort

sort: main.o getWord.o sort.o
	$(CC) $(COPTS) -o sort main.o getWord.o sort.o


main.o: main.c getWord.h
	$(CC) $(COPTS) -c main.c getWord.h
	
getWord.o: getWord.c getWord.h
	$(CC) $(COPTS) -c getWord.c getWord.h
	
sort.o: sort.c sort.h
	$(CC) $(COPTS) -c sort.c sort.h

clean: 
	rm $(JUNK)
