#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#define TAM 1000000

int main(int argc, char *argv[])
{
    int sockfd, puerto;
    struct sockaddr_in dest_addr;
    struct hostent *server;

    if (argc != 3)
    {
        printf("Uso: %s <server>, Puerto IPv4 a enviar\n", argv[0]);
        exit(1);
    }
    else if (atoi(argv[2]) < 1024 || atoi(argv[2]) > 65535)
    {
        printf("Puerto invalido, debe estar entre 1024 y 65535 y debe ser un valor numerico\n");
        exit(1);
    }

    server = gethostbyname(argv[1]);

    if(server == NULL)
    {
        printf("Error, no se pudo resolver el nombre\n");
        exit(1);
    }

    puerto = atoi(argv[2]);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons((uint16_t)puerto);
    dest_addr.sin_addr = *((struct in_addr *)server->h_addr);
    memset(&(dest_addr.sin_zero), '\0', 8);

    if (connect(sockfd, (struct sockaddr *)&dest_addr, sizeof(dest_addr)) < 0)
    {
        perror("connect");
        exit(1);
    }
    char buffer[TAM];
    while (1)
    {
        printf("\n>");
        memset(buffer, 0, sizeof(buffer));
        fflush(stdin);
        fgets(buffer, TAM, stdin);
        ssize_t n = send(sockfd, buffer, sizeof(buffer), 0);
        if (n <= 0)
        {
            perror("sendto");
            exit(1);
        }
        memset(buffer, 0, sizeof(buffer));
        n = recv(sockfd, buffer, sizeof(buffer), 0);
        printf("%s", buffer);
    }
    return 0;
}
