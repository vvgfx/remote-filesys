#
# Makefile / Practicum - 2 / Client
#
# Vishnu Vardan S / CS5600 / Northeastern University
# Fall 2025 / Dec 4, 2025
#

OBJS = utility.o get.o write.o rm.o ls.o get_version.o stop.o
CC = gcc
CFLAGS = -Wall -Wextra

all: client

clean:
	rm -f rfs *.o

utility.o: utility.h utility.c
	$(CC) $(CFLAGS) utility.c -c

get.o: get.h get.c
	$(CC) $(CFLAGS) get.c -c

write.o: write.h write.c
	$(CC) $(CFLAGS) write.c -c

rm.o: rm.h rm.c
	$(CC) $(CFLAGS) rm.c -c

ls.o: ls.h ls.c
	$(CC) $(CFLAGS) ls.c -c

get_version.o: get_version.h get_version.c
	$(CC) $(CFLAGS) get_version.c -c

stop.o: stop.h stop.c
	$(CC) $(CFLAGS) stop.c -c

client: $(OBJS) client.c client.h
	$(CC) $(CFLAGS) client.c -o rfs $(OBJS)