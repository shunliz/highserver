CFLAGS= -c -g
CC=gcc

server:server4.o log.o Locker.o locker_pthread.o
	$(CC) server4.o log.o Locker.o locker_pthread.o -LS -lpthread -o server

server4.o: server4.c
	$(CC) $(CFLAGS) server4.c

log.o:log.c log.h
	$(CC) $(CFLAGS) log.c

Locker.o: Locker.c Locker.h
	$(CC) $(CFLAGS) Locker.c

locker_pthread.o:locker_pthread.c locker_pthread.h
	$(CC) $(CFLAGS) locker_pthread.c 

./test/client1.o:./test/client1.c
	$(CC) $(CFLAGS) ./test/client1.c
	
clean:
	rm server4.o log.o Locker.o locker_pthread.o ./test/client1.o

test:./test/client1.o log.o Locker.o locker_pthread.o
	
	$(CC) ./test/client1.o log.o Locker.o locker_pthread.o -LS -lpthread -o ./test/client1
	
