#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <string.h>

#define TAM 10000


int main(int argc, char *argv[])
{
	int sockfd;
	struct sockaddr_un struct_cliente;

 if (argc != 2)
  {
    printf("Uso: %s <nombre_de_socket>", argv[0]);
    exit(1);
  }
	char query[] = "SELECT * From Cars";

	/* Creacion de socket */
	if ((sockfd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0)
		perror("socket");

	/* Inicialización y establecimiento de la estructura del cliente */
	memset(&struct_cliente, 0, sizeof(struct_cliente));
	struct_cliente.sun_family = AF_UNIX;
	strncpy(struct_cliente.sun_path, argv[1], sizeof(struct_cliente.sun_path));

	int child_pid;

	if ((child_pid = fork()) < 0)
	{
		perror("fork");
		exit(1);
	}
	if (child_pid == 0)
	{
		if (connect(sockfd, (struct sockaddr *)&struct_cliente, sizeof(struct_cliente)) < 0)
        {
            perror("connect");
            exit(1);
        }
		while (1)
		{
			//sleep(1);
			char buffer[TAM];
			memset(buffer, 0, sizeof(buffer));
            ssize_t n = send(sockfd, query, sizeof(query), 0);
			if (n < 0)
			{
				perror("sendto, es posible que el server no este levantado");
				exit(1);
			}
			n = recv(sockfd, buffer, TAM, 0);
			printf("%s\n", buffer);
			if (n < 0)
			{
				perror("recv");
				exit(1);
			}
		}
	}

	return 0;
}