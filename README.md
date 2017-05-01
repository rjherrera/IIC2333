# Tarea 2

#### Integrantes

```c
char *ap[2] = {"Pérez", "Herrera"};
for (int i = 0; i < 2; ++i) printf("%d) Raimundo %s\n", i + 1, ap[i]);
```

### Makefile

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

#### Juego de la vida


#### Memoria virtual