CFLAGS = -Wall -pedantic -Werror -Wextra -Wconversion -std=gnu11 

all: obj/servidor.o
	gcc $(CFLAGS) -o serv obj/servidor.o -lsqlite3
	cppcheck --enable=all ./bin style,performance,portability	
	
obj/servidor.o: bin/cliente-servidor/servidor.c
	mkdir -p obj
	gcc $(CFLAGS) -c bin/cliente-servidor/servidor.c -o obj/servidor.o -lsqlite3


clean:
	rm -f obj/*.o
	rm -f ipv4_log.txt ipv6_log.txt local_log.txt arch cli cli4 cli6 serv
	rmdir obj