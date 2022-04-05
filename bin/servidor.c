#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <string.h>
#include <sys/unistd.h>
#include <netinet/in.h>
#include <net/if.h>
#include <sys/mman.h>
#include <sys/prctl.h>
#include <signal.h>
#include <sqlite3.h>

#define TAM 10000

/*
Esta funcion realiza una query pasada en un buffer como parametro, contra la base de datos eligiendo al azar entre las 5 base de datos
disponibles pasadas en un array de estructuras sqlite3 como parametro. La funcion retorna un string 
con la respuesta de la base de datos. Si hubo un error, retorna el string con el error.
*/
char *query_db(char buffer[], char *query, sqlite3 *db[])
{
    int i = rand() % 5;
    sqlite3_stmt *res;
    int rc = sqlite3_prepare_v2(db[i], query, -1, &res, 0);
    if (rc != SQLITE_OK)
    {
      fprintf(stderr, "Error en el prepare: %i\n", rc);
      exit(1);
    }
    rc = sqlite3_step(res);
    if (rc == SQLITE_DONE)
    {
      strcat(buffer, "DONE");
    }
    else if (rc == SQLITE_ERROR)
    {
      fprintf(stderr, "Error en el step: %i\n", rc);
    }
    else if (rc == SQLITE_ROW)
    {
      do
      {
        int j = sqlite3_column_count(res);
        for (int i = 0; i < j; i++)
        {
          char *tmp = (char *)sqlite3_column_text(res, i);
          strncat(buffer, tmp, strlen(tmp));
          strcat(buffer, " ");
        }
        strcat(buffer, "\n");
      } while (sqlite3_step(res) == SQLITE_ROW);
    }
    sqlite3_finalize(res);
    return buffer;
}

/*
Esta funcion toma como parametro un buffer de caracteres y un long unsigned int. Luego convierte
la cadena de caracteres en un long unsigned int, le suma el otro numero tomado como parametro
y lo guarda en el buffer de caracteres.
*/
void agregar_cantidad_recibida(char *buffer, unsigned long int numero)
{
  unsigned long int numero_a_sumar = (long unsigned int)atoi(buffer);
  numero_a_sumar += numero;
  sprintf(buffer, "%lu", numero_a_sumar);
}

/*
Toma como argumento el path a un archivo, y un long int. Escribe el long int dejando una
linea respecto al contenido anterior del archivo. Si el archivo no existe, lo crea.
*/
void escribir_archivo(char *path, unsigned long int numero)
{
  FILE *archivo;
  archivo = fopen(path, "a");
  if (archivo == NULL)
  {
    printf("No se pudo abrir el archivo\n");
    exit(1);
  }
  fprintf(archivo, "Velocidad: %lu Mb/s\n", numero);
  fclose(archivo);
}

void *create_shared_memory(size_t size)
{
  int protection = PROT_READ | PROT_WRITE;
  int visibility = MAP_SHARED | MAP_ANONYMOUS;
  return mmap(NULL, size, protection, visibility, -1, 0);
}
void configurar_base_datos(sqlite3 *db)
{
  char *err_msg = 0;
  char *sql = "DROP TABLE IF EXISTS Cars;"
              "CREATE TABLE Cars(Id INT, Name TEXT, Price INT);"
              "INSERT INTO Cars VALUES(1, 'Audi', 52642);"
              "INSERT INTO Cars VALUES(2, 'Mercedes', 57127);"
              "INSERT INTO Cars VALUES(3, 'Skoda', 9000);"
              "INSERT INTO Cars VALUES(4, 'Volvo', 29000);"
              "INSERT INTO Cars VALUES(5, 'Bentley', 350000);"
              "INSERT INTO Cars VALUES(6, 'Citroen', 21000);"
              "INSERT INTO Cars VALUES(7, 'Hummer', 41400);"
              "INSERT INTO Cars VALUES(8, 'Volkswagen', 21600);";
  int rc = sqlite3_exec(db, sql, 0, 0, &err_msg);
  if (rc != SQLITE_OK)
  {
    fprintf(stderr, "SQL error: %s\n", err_msg);
    sqlite3_free(err_msg);
    sqlite3_close(db);
    exit(1);
  }
}
sqlite3* abrir_base_datos(sqlite3 *db)
{
  db = create_shared_memory((size_t) sqlite3_memory_used());
  int rc = sqlite3_open_v2("obj/BDD.db", &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_FULLMUTEX | SQLITE_OPEN_SHAREDCACHE, NULL );
  if (rc != SQLITE_OK)
  {
    fprintf(stderr, "No se puede abrir la base de datos: %s\n", sqlite3_errmsg(db));
    sqlite3_close(db);
    exit(1);
  }
  return db;
}

int main(int argc, char *argv[])
{
  int socket_server;
  struct sockaddr_un serv_local, cli_addr;
  struct sockaddr_in serv_addr4, cli_addr4;
  struct sockaddr_in6 serv_addr6, cli_addr6;

  void *local_buf = create_shared_memory(TAM);
  void *ipv4_buf = create_shared_memory(TAM); // buffers del mecanismo de shared memory
  void *ipv6_buf = create_shared_memory(TAM);
  memset(local_buf, 0, sizeof((char *)local_buf));
  memset(ipv4_buf, 0, sizeof((char *)ipv4_buf));
  memset(ipv6_buf, 0, sizeof((char *)ipv6_buf));

  if (argc != 4)
  {
    printf("Uso: %s <nombre_de_socket>, Puerto IPv4 a escuchar, Puerto IPv6 a escuchar\n", argv[0]);
    exit(1);
  }
  else if (atoi(argv[2]) < 1024 || atoi(argv[3]) < 1024)
  {
    printf("Puerto invalido, debe ser mayor o igual a 1024 y debe ser un valor numerico\n");
    exit(1);
  }
  else if (atoi(argv[2]) > 65535 || atoi(argv[3]) > 65535)
  {
    printf("Puerto invalido, debe ser menor o igual a 65535\n");
    exit(1);
  }
  else if (atoi(argv[2]) == atoi(argv[3]))
  {
    printf("Puertos iguales, deben ser distintos\n");
    exit(1);
  }

  if ((socket_server = socket(AF_UNIX, SOCK_STREAM, 0)) < 0)
  {
    perror("socket local");
    exit(1);
  }

  sqlite3 *db[5] = { NULL };
  db[0] = abrir_base_datos(db[0]);
  db[1] = abrir_base_datos(db[1]);
  db[2] = abrir_base_datos(db[2]);
  db[3] = abrir_base_datos(db[3]);
  db[4] = abrir_base_datos(db[4]);
  configurar_base_datos(db[0]);
  // configurar_base_datos(db[1]);
  // configurar_base_datos(db[2]);
  // configurar_base_datos(db[3]);
  // configurar_base_datos(db[4]);

  // A PARTIR DE ACA SE CREAN LOS SOCKETS

  unlink(argv[1]);

  memset(&serv_local, 0, sizeof(serv_local));
  serv_local.sun_family = AF_UNIX;
  strncpy(serv_local.sun_path, argv[1], sizeof(serv_local.sun_path));

  if ((bind(socket_server, (struct sockaddr *)&serv_local, (socklen_t)SUN_LEN(&serv_local))) < 0)
  {
    perror("ERROR en binding local, es posible que se haya ingresado un argumento invalido");
    exit(1);
  }

  printf("Socket disponible: %s\n", serv_local.sun_path);

  int child_pid;
  if ((child_pid = fork()) == -1)
  {
    perror("fork");
    exit(1);
  }
  if (child_pid == 0)
  {
    prctl(PR_SET_PDEATHSIG, SIGHUP);
    listen(socket_server, 1000);
    int clilen = sizeof(cli_addr);
    while (1)
    {
      int newsockfd = accept(socket_server, (struct sockaddr *)&cli_addr, (socklen_t *)&clilen);
      int children_client_pid;
      if ((children_client_pid = fork()) == -1)
      {
        perror("fork");
        exit(1);
      }
      if (children_client_pid == 0)
      {
        close(socket_server);
        while (1)
        {
          char recibido[TAM];
          char respuesta[TAM];
          memset(recibido, 0, sizeof(recibido));
          memset(respuesta, 0, sizeof(respuesta));
          long int resultado = read(newsockfd, recibido, TAM - 1);
          if (resultado < 0)
          {
            perror("recvfrom");
            exit(1);
          }
          agregar_cantidad_recibida((char *)local_buf, (unsigned long)resultado);

          query_db(respuesta, recibido, db);
          size_t respuesta_size = strlen(respuesta);
          resultado = write(newsockfd, respuesta, respuesta_size);
          if (resultado < 0)
          {
            perror("sendto");
            exit(1);
          }
        }
      }
    }
    return 0;
  }
  // para socket ipv4...
  if ((socket_server = socket(AF_INET, SOCK_STREAM, 0)) < 0)
  {
    perror("socket IPv4");
    exit(1);
  }

  memset(&serv_addr4, 0, sizeof(serv_addr4));
  int puerto = atoi(argv[2]);
  serv_addr4.sin_port = htons((uint16_t)puerto);
  serv_addr4.sin_family = AF_INET;
  serv_addr4.sin_addr.s_addr = INADDR_ANY;
  memset(&(serv_addr4.sin_zero), '\0', 8);

  if (bind(socket_server, (struct sockaddr *)&serv_addr4, sizeof(serv_addr4)) < 0)
  {
    perror("ERROR en binding IPv4, es posible que se haya ingresado un argumento invalido");
    exit(1);
  }

  printf("Socket disponible: %d\n", ntohs(serv_addr4.sin_port));

  if ((child_pid = fork()) == -1)
  {
    perror("fork");
    exit(1);
  }

  if (child_pid == 0)
  {
    prctl(PR_SET_PDEATHSIG, SIGHUP);
    listen(socket_server, 1000);
    int clilen = sizeof(cli_addr4);
    while (1)
    {
      int newsockfd = accept(socket_server, (struct sockaddr *)&cli_addr4, (socklen_t *)&clilen);
      int children_client_pid;
      if ((children_client_pid = fork()) == -1)
      {
        perror("fork");
        exit(1);
      }
      if (children_client_pid == 0)
      {
        close(socket_server);
        while (1)
        {
          char buffer[TAM];
          memset(buffer, 0, TAM);
          long int resultado = read(newsockfd, buffer, TAM - 1);
          if (resultado < 0)
          {
            perror("read");
            exit(1);
          }
          agregar_cantidad_recibida((char *)ipv4_buf, (unsigned long)resultado);
        }
      }
    }
  }
  // 3er protocolo
  if ((socket_server = socket(AF_INET6, SOCK_STREAM, 0)) < 0)
  {
    perror("socket IPv6");
    exit(1);
  }

  memset(&serv_addr6, 0, sizeof(serv_addr6));
  puerto = atoi(argv[3]);
  serv_addr6.sin6_port = htons((uint16_t)puerto);
  serv_addr6.sin6_family = AF_INET6;
  serv_addr6.sin6_addr = in6addr_any;

  if (bind(socket_server, (struct sockaddr *)&serv_addr6, sizeof(serv_addr6)) < 0)
  {
    perror("ERROR en binding IPv6, es posible que se haya ingresado un argumento invalido");
    exit(1);
  }

  printf("Socket disponible: %d\n", ntohs(serv_addr6.sin6_port));

  if ((child_pid = fork()) == -1)
  {
    perror("fork");
    exit(1);
  }
  if (child_pid == 0)
  {
    prctl(PR_SET_PDEATHSIG, SIGHUP);
    listen(socket_server, 1000);
    int clilen = sizeof(cli_addr6);
    while (1)
    {
      int newsockfd = accept(socket_server, (struct sockaddr *)&cli_addr6, (socklen_t *)&clilen);
      int children_client_pid;
      if ((children_client_pid = fork()) == -1)
      {
        perror("fork");
        exit(1);
      }
      if (children_client_pid == 0)
      {
        close(socket_server);
        while (1)
        {
          char buffer[TAM];
          memset(buffer, 0, TAM);
          long int resultado = read(newsockfd, buffer, TAM - 1);
          if (resultado < 0)
          {
            perror("recvfrom");
            exit(1);
          }
          agregar_cantidad_recibida((char *)ipv6_buf, (unsigned long)resultado);
        }
      }
    }
  }
  // en esta seccion de codigo se queda el proceso padre registrando las velocidades
  // de cada protocolo
  while (1)
  {
    sleep(1);

    long unsigned int velocidad_local = (long unsigned int)atoi(local_buf) / 131072;
    long unsigned int velocidad_ipv4 = (long unsigned int)atoi(ipv4_buf) / 131072;
    long unsigned int velocidad_ipv6 = (long unsigned int)atoi(ipv6_buf) / 131072;

    printf("Velocidad local: %lu Mb/s\n", velocidad_local);
    printf("Velocidad IPv4: %lu Mb/s\n", velocidad_ipv4);
    printf("Velocidad IPv6: %lu Mb/s\n", velocidad_ipv6);

    memset(local_buf, 0, sizeof((char *)local_buf));
    memset(ipv4_buf, 0, sizeof((char *)ipv4_buf));
    memset(ipv6_buf, 0, sizeof((char *)ipv6_buf));

    escribir_archivo("local_log.txt", velocidad_local);
    escribir_archivo("ipv4_log.txt", velocidad_ipv4);
    escribir_archivo("ipv6_log.txt", velocidad_ipv6);

    fprintf(stderr, "\n\n\n");
  }
  return 0;
}
