CC= gcc
CFLAGS= -std=c99 -Wall -g

all: master slave
files: master slave

master: master.c
	$(CC) $(CFLAGS) -o master master.c 

slave: slave.c
	$(CC) $(CFLAGS) -o slave slave.c 

clean:
	rm -f master
	rm -f slave