# Socket implementado en C siguiendo las directrices principales del RFC 3977, funcional en Ubuntu 18.04 y en Debian

Se implementa un servidor que proporciona algunas de las funcionalidades del protocolo NNTP (detalladas más adelante).
Dicho servidor puede actuar tanto en TCP como en UDP.

## TCP
El servidor realiza la secuencia de la API de sockets para lograr establecer un puerto de escucha, en este caso en el 8438 (este valor se puede cambiar), atenderá las peticiones de los clientes siguiendo el protocolo TCP

## UDP
El servidor permite conexiones mediante UDP por el puerto indicado, cuando recibe una nueva conexión genera un nuevo socket en un puerto aleatorio mediante el que se comunicará de manera exclusiva con el cliente. El cliente debe anotar la direccion del puerto por el que ha recibido la respuesta.

## Ordenes implementadas
<ul>
  <li>LIST</li>
  <li>GROUP</li>
  <li>ARTICLE</li>
  <li>HEAD</li>
  <li>BODY</li>
  <li>NEWGROUPS</li>
  <li>NEWNEWS</li>
  <li>POST</li>
</ul>

El funcionamiento de todas estas ordenes se encuentra en el fichero en el que están implementadas, serverUtils.c

## Ficheros
<ul>
  <li>articulos/local: directorios donde se encuentran las carpetas con los distintos grupos</li>
  <li>informe: informe realizado y necesario para la entrega de la práctica</li>
  <li>Makefile: fichero encargado de ejecutar las ordenes de compilación tanto del servidor como de los clientes</li>
  <li>cliente.c: codigo del cliente, puede ejecutarse en modo TCP o en modo UDP</li>
  <li>cola.c: TAD cola necesario para procesar las distintas órdenes en el servidor</li>
  <li>grupos: fichero en el que se almacena el nombre de todos los grupos presentes, el número de el primer y el último artículo que contienen, fecha y hora de creación y una breve descripción</li>
  <li>lanzaServidor.sh / lanzaServidor2.sh: shell scripts diseñados para lanzar el servidor en primer plano y los clientes en segundo plano. La existencia de lanzaServidor2.sh se debe a un error en el que no lanzaba el servidor y este debía ser ejecutado previamente al script</li>
  <li>n_articulos: fichero que guarda el número de artículos presentes en el sistema</li>
  <li>ordenesN.txt: ficheros que contienen las diferentes órdenes ejecutadas por los clientes.</li>
  <li>serverUtils.c: fichero en el que se implementan las distintas órdenes que procesa el servidor. Se decide implementarlas en otro fichero para fomentar la modularidad</li>
  <li>servidor.c: fichero con el código del servidor</li>
</ul>

## Notas y comentarios

- Es recomendable usar un timeout a la hora de recibir peticiones en UDP para evitar bloquear al proceso infinitamente. En este ejemplo no se implementa para el servidor ni para los clientes.
- Los clientes permiten un modo de ejecución manual, en el que se pueden introducir las órdenes por la consola, para ejecutar el modo manual sirve con ejecutar el servidor previamente y a la hora de ejecutar el cliente omitir el parámmetro que indica el fichero de órdenes.



