CFLAGS= -c -g
CC=gcc

server9:server9.o log.o Locker.o locker_pthread.o
	$(CC) server9.o log.o Locker.o locker_pthread.o -LS -lpthread -o server9

server9.o: server9.c
	$(CC) $(CFLAGS) server9.c

log.o:log.c log.h
	$(CC) $(CFLAGS) log.c

Locker.o: Locker.c Locker.h
	$(CC) $(CFLAGS) Locker.c

locker_pthread.o:locker_pthread.c locker_pthread.h
	$(CC) $(CFLAGS) locker_pthread.c 

client9.o:client9.c
	$(CC) $(CFLAGS) client9.c
	
clean:
	rm server9.o log.o Locker.o locker_pthread.o client9.o

client9:client9.o log.o Locker.o locker_pthread.o
	
	$(CC) client9.o log.o Locker.o locker_pthread.o -LS -lpthread -o client9
	
