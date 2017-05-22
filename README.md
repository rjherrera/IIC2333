# My project's README

Consideraciones y Supuestos

Cuando una instruccion ha sido ignorada, no se considera como una ejecución para propósitos de steps.

Siempre se comienza la simulación inicializando el sistema de archivos en un 'ROOT' que es un directorio de nombre "base_dir". Independiente de si se decidió cargar un disco o no, este directorio sobreescribe el primer bloque del disco.

Cuando se mueve un directorio se escriben todos los nuevos accesos en el archivo 'accesos.txt'. Siguiendo lo que se mencionó en el foro, no importa que aparezcan directorios que ya no existen, pues no habrá un bloque que apunte a la línea respectiva.

Se asumió que rm recibe una ruta relativa para eliminar.

Cuando se elimina un archivo o directorio, el bloque asociado se resetea al estado inicial. (Solo bit de bloque libre seteado.)

Se asume también que la posición de escritura indicada está dentro del espacio que tiene el archivo, y que es mayor o igual a 1.

Se asumió que si se reducía el tamaño de un archivo, en el peor de los casos quedaría con 0 Bytes, pero sus 50B virtuales seguirían existiendo. También se asumió que el número de bytes a eliminar está dentro de los rangos posibles, asumiendo que no se pueden borrar los primeros 50B virtuales.

Se asume que cada instrucción de 'acciones.txt' siempre parte con una keyword para la instrucción, seguida de un espacio antes de los parámetros.