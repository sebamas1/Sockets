CFLAGS = -Wall -pedantic -Werror -Wextra -Wconversion -std=gnu11 

serv: obj/servidor.o
	gcc $(CFLAGS) -o serv $^ -lsqlite3
	
obj/servidor.o: bin/servidor.c
	mkdir -p obj
	gcc $(CFLAGS) -c bin/servidor.c -o obj/servidor.o -lsqlite3


clean:
	rm -f obj/*.o
	rm -f ipv4_log.txt ipv6_log.txt local_log.txt arch cli cli4 cli6 serv
	rmdir obj