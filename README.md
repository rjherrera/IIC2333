# Consideraciones T4

Para las conexiones se procedió de acuerdo a lo indicado en la ayudantía y el enunciado.
Se envían los mensajes correspondientes y las diversas formas de enviarlos están implementadas

Para el parseo de argumentos se procedió como en el enunciado, es decir con -l opcional que indica que el que ejecuta es el servidor, con -i opcional solo si es el servidor, y si no lo pone se asigna ANY.

### Bonus

Se implementó el bonus realizado con archivos, lo que se realizó fue crear un archivo para cada usuario e ir actualizandolo. Si el usuario termina el programa inesperadamente, por la razón que sea, este se guarda y deja registrado que "hay que recuperar", y en la siguiente partida se continua desde donde se dejó.

Para evitar que solo se acabe legalmente el juego al ganar, se implementó la señal ctrl + z de modo que si se desea salir en cualquier momento y que la proxima vez que se juegue no se recargue estado anterior, entonces se puede usar ctrl+z.

Con ctrl+c o cualquier otra "botada" del juego va a quedar registrado que si hay que restaurar el juego.

Los archivos que se generan para el bonus son .btsp y solo son modificados con estos fines.

#### Consideración fflush

El implementar fflush para evitar que lo que se escribiese al estar esperando el turno no funcionó como es esperado, al parecer el guardado en el stdin es responsabilidad del bash y no de nuestro programa y no había como capturarlo, tras hablarlo con el profesor, comentó que sabía de este problema y que lo comentaramos acá para evitar descuentos.