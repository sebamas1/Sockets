CFLAGS = -Wall -pedantic -Werror -Wextra -Wconversion -std=gnu11 

all : serv cli cli4

serv: obj/servidor.o
	gcc $(CFLAGS) -o serv obj/servidor.o -lsqlite3

cli: obj/cliente.o
	gcc $(CFLAGS) -o cli obj/cliente.o

cli4: obj/cliente4.o
	gcc $(CFLAGS) -o cli4 obj/cliente4.o
	
obj/servidor.o: bin/servidor.c
	mkdir -p obj
	gcc $(CFLAGS) -c bin/servidor.c -o obj/servidor.o -lsqlite3

obj/cliente.o: bin/cliente.c
	mkdir -p obj
	gcc $(CFLAGS) -c bin/cliente.c -o obj/cliente.o -lsqlite3

obj/cliente4.o: bin/cliente4.c
	mkdir -p obj
	gcc $(CFLAGS) -c bin/cliente4.c -o obj/cliente4.o -lsqlite3

clean:
	rm -f obj/*
	rm -f ipv4_log.txt ipv6_log.txt local_log.txt arch cli cli4 cli6 serv
	rmdir obj