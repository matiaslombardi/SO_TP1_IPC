CC= gcc
CFLAGS= -std=c99 -Wall -g
AUX_FLAGS = -lrt -pthread

all: master slave view

master: master.c
	$(CC) $(CFLAGS) -o master master.c $(AUX_FLAGS)

slave: slave.c
	$(CC) $(CFLAGS) -o slave slave.c 

view: view.c
	$(CC) $(CFLAGS) -o view view.c $(AUX_FLAGS)

test:
	make all
	valgrind --leak-check=full -v --show-leak-kinds=all ./master Files/* > ./view 2> master.valgrind
	cppcheck --quiet --enable=all --force --inconclusive --suppress=missingIncludeSystem . 2> cppoutput.txt
	make clean
	pvs-studio-analyzer trace -- make > /dev/null
	pvs-studio-analyzer analyze > /dev/null 2> /dev/null
	plog-converter -a '64:1,2,3;GA:1,2,3;OP:1,2,3' -t tasklist -o report.tasks PVS-Studio.log > /dev/null
	
.PHONY: clean clean_test clean_all
clean:
	rm -f master
	rm -f slave
	rm -f view
clean_test:
	rm -f master.valgrind
	rm -f cppoutput.txt
	rm -f report.tasks
	rm -f PVS-Studio.log
	rm -f strace_out
clean_all:
	make clean
	make clean_test