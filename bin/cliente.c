#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <string.h>


static char *rand_string(char *str, size_t size)
{
	const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJK...";
	if (size)
	{
		--size;
		for (size_t n = 0; n < size; n++)
		{
			int key = rand() % (int)(sizeof charset - 1);
			str[n] = charset[key];
		}
		str[size] = '\0';
	}
	return str;
}

int main(int argc, char *argv[])
{
	int sockfd;
	struct sockaddr_un struct_cliente;

 if (argc != 3)
  {
    printf("Uso: %s <nombre_de_socket>, Cantidad de Bytes a enviar por mensaje", argv[0]);
    exit(1);
  }
  else if (atoi(argv[2]) > 10000)
  {
    printf("Solo se pueden enviar mensajes de un maximo de 10000 Bytes\n");
    exit(1);
  }

	size_t bytes_enviar = (size_t)atoi(argv[2]);

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
		char buffer[bytes_enviar];
		while (1)
		{
			memset(buffer, 0, bytes_enviar);
            ssize_t n = write(sockfd, rand_string(buffer, bytes_enviar), bytes_enviar);
			if (n < 0)
			{
				perror("sendto, es posible que el server no este levantado");
				exit(1);
			}
		}
	}

	return 0;
}