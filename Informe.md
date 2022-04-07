# Informe TP2

## Desarrollo

Se implementaron 5 estructuras sqlite3 que son inicializadas con la misma base de datos, y esta base de datos es cargada con algunos valores por defecto. Esto cumple el requisito de que el servidor tenga 5 conexiones contra la base de datos. Se aceptan cualquier numero de clientes conectados tipo A y B, y a cada cliente se le asigna una de las conexiones al azar. Sin embargo si se levantan muchos clientes la base de datos empieza a responder muy lento. En cuando a los clientes tipo C puede tambien haber N cantidad de este tipo de clientes, pero no se les asigna una conexion contra la DB sino que directamente se les pasa una copia de la base de datos que es almacenada en **obj/BDD1.db**.

Para la comunicacion de los subprocesos contra la base de datos se implemento un sistema de memoria compartida.

## Problemas encontrados

Al principio del desarrollo tuve problemas haciendo querys contra la base de datos, y esto se debia a que cuando realizaba un *SELECT* no realizaba un STEP la cantidad de veces que se requerian, por lo que a la siguiente vez que ingresaba a la base de datos se rompia el programa. Este es el problema que por lejos mas tiempo me tomo, y se debia principalemente a no saber usar la libreria propuesta, y de no tener en cuenta las FLAGS de retorno que da la libreria. Al empezar a tomar en cuenta estas banderas, el proyecto empezo a avanzar mucho mas rapido.

## Uso

Se utiliza make para construir tanto los clientes como el servidor
~~~
make
~~~

Luego se levanta el servidor:
~~~
./serv ./arch 1234 1235
~~~

Estos valores se los pasa asi porque los scripts estan escritos con esos valores por defecto.

Luego, para levantar clientes tipo A se usa el siguiente comando.

~~~
bash clienteAF.sh
~~~
Este script levanta 50 clientes que piden la tabla entera y la imprimen por pantalla. Aunque el programa funciona correctamente levantando estos 50 clientes y luego levantar clientes tipo B, se recomienda levantar 1 cliente de cada uno ya que el programa se torna muy lento.

Para levantar 10 clientes que realizan un INSERT se puede usar el script

~~~
bash clienteipv4.sh
~~~

Tambien se pueden levantar varios clientes tipo C, pero estos solo sobreescribiran la db que el anterior cliente tipo C escribio, asi que no tiene mucho sentido levantar muchos clientes tipo C

~~~
bash clienteipv6.sh
~~~



