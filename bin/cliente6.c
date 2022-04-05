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
    int sockfd, puerto;
    struct sockaddr_in6 dest_addr;
    struct hostent *server;

    if (argc != 4)
    {
        printf("Uso: %s <nombre_de_socket>, Puerto IPv6 a enviar, Cantidad de Bytes a enviar por mensaje\n", argv[0]);
        exit(1);
    }
    else if (atoi(argv[2]) < 1024 || atoi(argv[2]) > 65535)
    {
        printf("Puerto invalido, debe estar entre 1024 y 65535 y debe ser un valor numerico\n");
        exit(1);
    }
    else if (atoi(argv[3]) > 10000)
    {
        printf("Solo se pueden enviar mensajes de un maximo de 10000 Bytes\n");
        exit(1);
    }

    size_t bytes_enviar = (size_t)atoi(argv[3]);

    server = gethostbyname2(argv[1], AF_INET6);

    puerto = atoi(argv[2]);
    sockfd = socket(AF_INET6, SOCK_STREAM, 0);

    dest_addr.sin6_family = AF_INET6;
    dest_addr.sin6_port = htons((uint16_t)puerto);
    dest_addr.sin6_addr = *((struct in6_addr *)server->h_addr);
    int child_pid;

    if ((child_pid = fork()) < 0)
    {
        perror("fork");
        exit(1);
    }

    if (child_pid == 0)
    {
        if (connect(sockfd, (struct sockaddr *)&dest_addr, sizeof(dest_addr)) < 0)
        {
            perror("connect");
            exit(1);
        }
        while (1)
        {
            char buffer[bytes_enviar];
            memset(buffer, 0, bytes_enviar);
            ssize_t n = write(sockfd, rand_string(buffer, bytes_enviar), bytes_enviar);
            // printf("Bytes enviados: %d\n", n);
            if (n < 0)
            {
                perror("sendto");
                exit(1);
            }
        }
    }
    return 0;
}