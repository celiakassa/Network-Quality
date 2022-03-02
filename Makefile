CFLAGS=-Wall -O2 -Werror -Wall -Wextra -Wpedantic -Wformat=2 -Wformat-overflow=2 -Wformat-truncation=2 -Wformat-security -Wnull-dereference -Wstack-protector -Wtrampolines -Walloca -Wvla -Warray-bounds=2 -Wimplicit-fallthrough=3  -Wshift-overflow=2 -Wcast-qual -Wstringop-overflow=4 -Wconversion -Warith-conversion -Wlogical-op -Wduplicated-cond -Wduplicated-branches -Wformat-signedness -Wshadow -Wstrict-overflow=4 -Wundef -Wstrict-prototypes -Wswitch-default -Wswitch-enum -Wstack-usage=1000000 -Wcast-align=strict -D_FORTIFY_SOURCE=2 -fstack-protector-strong -fstack-clash-protection -fPIE -Wl,-z,relro -Wl,-z,now -Wl,-z,noexecstack -Wl,-z,separate-code

all: rpm.o task.o shm.o test.o worker netquality 


rpm.o: rpm.c
	gcc -c rpm.c -o rpm.o
	
task.o: task.c
	gcc -c task.c -o task.o 
	
shm.o: shm.c
	gcc -c shm.c -o shm.o 

tworker.o: tworker.c
	gcc  -c tworker.c -o tworker.o -lssl -lcrypto
	
test.o: test.c
	gcc -c test.c -o test.o
	
netquality: tworker.o shm.o task.o rpm.o
	gcc -o networkqualityC  shm.o task.o rpm.o test.o main.c -lssl -lcrypto

worker: worker.c
	gcc  -o worker shm.o worker.c -lssl -lcrypto
	
test: netquality
	#valgrind --leak-check=full --track-origins=yes  --show-leak-kinds=all -s ./networkqualityC https://monitor.uac.bj:4449 large
	./networkqualityC https://monitor.uac.bj:4449 large
	
clean:
	rm *.o
	rm networkqualityC 
	rm worker
