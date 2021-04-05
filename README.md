# SO_TP1_IPC

## Autores
-Arce Doncella , Julian Francisco 60509                         
-Domingues, Paula Andrea 60148                                
-Lombardi, Matias Federico 60527  

## Instrucciones de Compilación
Ubicado dentro del directorio donde se descargaron los archivos, ejecutar el comando: 

     $ make all 

Esto generará todos los archivos ejecutables necesarios. 

## Instrucciones de Ejecución
El programa master guardará todos los resultados en un archivo, sin necesidad de otro proceso, para ello, ejecute:

     $ ./master ./Files/* 

Adicionalmente, se puede ejecutar el proceso view, que se encargará de imprimir por salida estándar los resultados, de la siguiente manera:

     $ ./view <MEMORY_SIZE>

donde el  valor <MEMORY_SIZE> será indicado por salida estándar al ejecutar el proceso master. Para ello, se cuenta con 2 segundos de espera.
Para iniciar ambos procesos a la vez, ejecutar lo siguiente:

     $ ./master ./Files/* | ./view 

## Instrucciones de Testeo y Limpieza
Finalmente, en el caso de querer realizar los testeos utilizando Valgrind, cppcheck y pvsStudio Analyzer basta con ejecutar, siempre y cuando nos encontremos en un entorno con las respectivas imágenes instaladas

     $ make test 

Lo cual generará tres archivos de lectura, uno para cada rutina de testeos ejecutada, master.valgrind el cual contará con el output de Valgrind, cppoutput.txt correspondiente al cppcheck y report.tasks, el cual corresponde a pvs-studio. 
Adicionalmente, en caso que se quisieran eliminar estos archivos se dispone de las siguientes opciones: 

     $ make clean 

para eliminar los tres ejecutables creados por el make all, 
    
     $ make clean_test 

para eliminar los archivos de texto creados por el make test, y por último

     $ make clean_all 

la cual engloba los dos clean anteriormente mencionados.  
