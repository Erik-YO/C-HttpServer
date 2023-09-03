
# Introducción

Este es el desarrollo de una práctica para la asignatura de REDES (2022).  
El objetivo es diseñar e implementar en lenguaje C un servidor web concurrente capaz de soportar peticiones simultáneas procedentes de varios clientes, con algunas limitaciones, pero plenamente funcional. Además, podrá responder a peticiones HTTP de tipo GET, POST y OPTIONS. 
El servidor web se configura a través de un fichero de configuración (server.conf) y acepta scripts de Python y php.


# Explicación de uso
## Configuración del servidor web
Principalmente, si se quiere cambiar algo sobre la configuración del servidor web, se tendrá que acceder al fichero de configuración server.conf y cambiar lo que se desea de estos datos:
>debug=false
>debug_file= stderr
>max_clients =40
>
>listen_port = 8080
>
>server_root=  ./www/ 
>
>server_signature  = Signature 1.1  
>
>max_threads=5

Uso de las variables:

-_debug_: si se quiere activar la impresión de mensajes de depuración. Por defecto este tendrá el valor “false”, no obstante, si se quiere activar, tendrá que ponerse a “true”

-_debug_file_: de dónde saldrá el resultado de la depuración. Por defecto está en “stderr”, para que los mensajes de error salgan en dicho canal

-_max_clients_: número máximo de clientes que puede atender a la vez (longitud de la cola de espera)

-_listen_port_: puerto de acceso al servidor

-_server_root_: directorio raíz del servidor

-_server_signature_: firma del servidor

-_max_threads_: número máximo de hilos que pueden existir a la vez (además del hilo principal)

## Ejecución
Para ejecutar el servidor, basta con ir a la terminal y usar el comando “make” para que se ejecute el servidor web creado en esta práctica. 
Entonces los clientes ya podrán hacer uso del servidor. Por ejemplo, para conectarnos desde un cliente navegador, podemos abrir una pestaña nueva de Chrome o Firefox e introducir el enlace para ver la página: “http://0.0.0.0:8080/index.html”  (0.0.0.0 se refiere a la ruta predetermina, que es el host local)

Para comprobar que efectivamente funciona, se han hecho pruebas con todo tipo de archivos (imágenes, gifs, textos planos, html o pdf…) y scripts de prueba. Por ejemplo, el cliente puede usar el script de prueba mediante el enlace “http://0.0.0.0:8080/scripts/test.py”


# Desarrollo técnico
En primer lugar, se hizo un programa muy sencillo que permitiera la comunicación entre un cliente y servidor a través de sockets. Para que no se perdiera la información, se decidió usar el protocolo TCP.

En paralelo a la construcción del servidor iterativo, había que pensar qué decisión tomar para hacerlo concurrente. Entre las distintas opciones que existía, se decidió elegir la implementación de un sistema multihilo al buscarse el rendimiento más efectivo. Esto significa permitir varios hilos en un solo proceso, ya que, como estos comparten memoria y archivos en un mismo proceso, se permite la comunicación mutua sin tener que invocar al núcleo. Por ende, también lleva menos tiempo crear o destruir un hilo (para lo cual creamos un módulo de gestión de hilos).
La opciones que contemplamos fueron: pool de hilos, pool de procesos, crear un proceso para cada petición o crear un hilo para cada petición. Se descartó la creación de un nuevo proceso por cada petición por motivos de rendimiento a pesar de ser la opción más fácil de implementar. También descartamos las opciones que requerían un pool porque la creación de un hilo por cada petición resultó ser más eficiente.

Volviendo al primer párrafo, tras conseguir la construcción de los hilos por separado, nos concentramos en construir un servidor web iterativo funcional para luego añadirle los hilos y hacerlo concurrente. Tras lograr parsear las peticiones HTTP con la librería picohttpparser que se ofrecía como opción en Moodle, el siguiente paso fue lograr que aceptara peticiones POST, GET y OPTIONS. Para el funcionamiento de esta parte, se hicieron numerosas pruebas usando un navegador web como cliente. Nosotros usamos Google Chrome y Firefox.

En el proceso, se creó el módulo Config, que sirve para centralizar la configuración del servidor web y facilitar la depuración, permitiendo ver o no los mensajes de error. 

Finalmente, tras ver que funcionaba el servidor web iterativo, se unió con los hilos, creando así un servidor web concurrente. Entonces se hicieron de igual manera pruebas, comprobando que fuera funcional y soportara los métodos HTTP anteriormente mencionados.


# Conclusiones

Se ha podido aprender mucho sobre el funcionamiento de los métodos HTTP y la comunicación entre cliente-servidor por medio de sockets. 

Probablemente la dificultad más grande fue a la hora de que funcionaran los métodos GET y POST, especialmente el segundo, ya que tenía que captar correctamente lo que se pedía. Luego la segunda dificultad sería unificar todo y hacer que funcionara bien a la hora de pasar de un servidor iterativo a uno concurrente.


---
Autores:

-Erik Yuste <erik.yuste@estudiante.uam.es>

-Shenxiao Lin <shenxiao.lin@estudiante.uam.es>
