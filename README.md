### Consideraciones

#### Tiempo de simulación
Para poder probar el funcionamiento de hacer Ctrl + C, lo que se implementó fue el uso de la función usleep la cual recibe como único argumento la cantidad de microsegundos a esperar. Por defecto se esperan 1E3 microsegundos, para alcanzar a hacer Ctrl + C pero no retardar excesivamente la ejecución del programa. Esta cantidad está definida como una constante al inicio del programa.

#### Orden de revisiones
Se considera en nuestro programa, cuando dos eventos se van a realizar en un mismo instante, como es el caso de la creación de procesos y el paso de waiting a ready, que la creación ocurre primero que el paso de waiting a ready.

#### Cantidad de bloqueos
Para la cantidad de bloqueos a los que es sometido un proceso, se consideraron todas las veces en las que un proceso pasó de running a waiting, sin embargo, para el caso especial de round robin, cuando un proceso gastaba su quantum y el scheduler le quitaba la CPU, no lo consideramos bloqueo.

#### Waiting time
Para el waiting time, se consideró todo el tiempo en el que el proceso no estuvo corriendo, es decir, waiting + ready.