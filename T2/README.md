# Tarea 2

#### Integrantes

```c
char *ap[2] = {"Pérez", "Herrera"};
for (int i = 0; i < 2; ++i) printf("%d) Raimundo %s\n", i + 1, ap[i]);
```

#### Makefile

Se incluye un makefile capaz de compilar todos los programas a la vez, para luego utilizar cada uno con su respectivo comando, es decir:

```
./msh
./lifegame <params>
./mmu
```

#### MSH

Se compila con:
```
gcc -o msh msh.c
```

Y se ejecuta con:
```
./msh
```

Casi no se incurrió en supuestos adicionales a los de la tarea, solo que cuando se hace CTRL+C en MSH y habían hijos corriendo en background se termina a MSH y a los hijos, por el contrario si hay un hijo en foreground, se termina solo al hijo y se vuelve al control de MSH.

Se hicieron avances en el bonus pero no se alcanzó a terminar

#### Juego de la vida / Lifegame

El programa comienza creando tantos procesos hijos como cpus, asignando a cada uno un sector distinto del mapa. Si hay menos filas y columnas que cpus, se crean tantos procesos hijos como filas o columnas. Para la simulación se comparte un arreglo de tantos bits como procesos hijos, y cada vez que un hijo termina su trabajo en la iteración, deja su bit en 0. Además, cada proceso tiene un contador de iteración personal que compara con la variable compartida de iteración, para no ejecutar cuando la iteración no es la que corresponde. El proceso padre revisa el arreglo compartido y cuando se da cuenta que todos los hijos han terminado su trabajo, avanza la iteración y vuelve a setear los bits en 1, para permitir que uno de los procesos hijo pueda avanzar. El primero que avanza abre el paso al resto de los hijos, y así sucesivamente. Lo anterior representa toda la sincronización que fue necesaria, pues para modelar el problema se utilizaron dos matrices. En cada iteración los hijos leen una matriz y escriben en la otra matriz, por lo tanto no hay conflicto entre ellos.

#### Memoria virtual

Para esta parte de la tarea se crearon los arreglos correspondientes muy similarmente a lo hablado en la ayudantía y se procedió a administrar la memoria, la estructura es un while que luego de acceder a lo deseado, imprime el valor de lo accedido en cada iteración, si se está ejecutando uno por uno, se introdujo manejo de señales para que si se hace CTRL+C se imprimiera la estadística actual. Al final del programa, si es llamado así:
```
./mmu < inputfile.txt
```
Se imprimen las estadísticas, que corresponden a hit rate y pagefault rate.

Se suprimieron los prints del programa según lo comentado en el foro, pero están comentados, para hacer debugging se pueden descomentar. La estructura es:

VirMem: XXX Page: XXX Offset: XXX
  miss-pagefault-hit
  frame: ZZZ
  value: YYY