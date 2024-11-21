Este proyecto tiene como objetivo desarrollar habilidades de programación multihebra y el uso de
mecanismos de sincronización. También permite comprender conceptos de memoria virtual mediante
la implementación de algoritmos de reemplazo de página.

Integrantes:
Franchesca Mora Chehuan (2021424903)
Brendan Rubilar Vivanco (2021750657) 
Tomás Cid Muñoz (2021454047) 
Luciano Argomedo (2021422935)


Ejemplo de ejecución para circularBuffer.cpp: $./simulapc -p 10 -c 5 -s 50 -t 1
Retorna un archivo log, con el proceso y actualizaciones del tamaño de la cola.

Ejemplo de ejecución para simuladorMV.cpp: ./mvirtual -m 3 -a FIFO -f archivo.txt
Se debe dar un archivo con un caso a evaluar, por ejemplo:
"0 1 3 4 1 2 5 1 2 3 4"
"0 1 2 3 4 0 1 2 3 4 5 6 7 8 9 5 6 7 8 9"

