### Consideraciones

#### Cantidad de bloqueos
Para la cantidad de bloqueos a los que es sometido un proceso, se consideraron todas las veces en las que un proceso pasó de running a waiting, sin embargo, para el caso especial de round robin, cuando un proceso gastaba su quantum y el scheduler le quitaba la CPU, también lo consideramos bloqueo.

#### Waiting time
Para la waiting time, se consideró todo el tiempo en el que el proceso no estuvo corriendo, es decir, waiting + ready.