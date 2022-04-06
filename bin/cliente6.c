#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/wait.h>
#include <sys/prctl.h>
#include <fcntl.h>

#define TAM 1000000

void write_file(int sockfd)
{
    remove("obj/BDD1.db");
    int fd;
    char *filename = "obj/BDD1.db";
    char buffer[TAM];

    fd = open(filename, O_CREAT | O_RDWR | O_APPEND, S_IRUSR | S_IWUSR | O_TRUNC);
    if (fd == -1)
    {
        perror("Error opening file");
        exit(1);
    }
    ssize_t n = recv(sockfd, buffer, TAM, 0);
    if (n <= 0)
    {
        perror("recv");
        exit(EXIT_FAILURE);
    }
    write(fd, buffer, (size_t) n);
    bzero(buffer, TAM);
    return;
}

int main(int argc, char *argv[])
{
    int sockfd, puerto;
    struct sockaddr_in6 dest_addr;
    struct hostent *server;

    if (argc != 3)
    {
        printf("Uso: %s <server>, Puerto IPv6 a enviar\n", argv[0]);
        exit(1);
    }
    else if (atoi(argv[2]) < 1024 || atoi(argv[2]) > 65535)
    {
        printf("Puerto invalido, debe estar entre 1024 y 65535 y debe ser un valor numerico\n");
        exit(1);
    }

    server = gethostbyname2(argv[1], AF_INET6);
    if(server == NULL)
    {
        printf("Error, no se pudo resolver el nombre\n");
        exit(1);
    }

    puerto = atoi(argv[2]);
    sockfd = socket(AF_INET6, SOCK_STREAM, 0);

    dest_addr.sin6_family = AF_INET6;
    dest_addr.sin6_port = htons((uint16_t)puerto);
    dest_addr.sin6_addr = *((struct in6_addr *)server->h_addr);

    if (connect(sockfd, (struct sockaddr *)&dest_addr, sizeof(dest_addr)) < 0)
    {
        perror("connect");
        exit(1);
    }
    write_file(sockfd);
    return 0;
}