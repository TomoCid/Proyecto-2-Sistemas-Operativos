# **Proyecto de Programación Multihebra y Memoria Virtual** 
Este proyecto tiene como objetivo desarrollar habilidades de programación multihebra y el uso de
mecanismos de sincronización. También permite comprender conceptos de memoria virtual mediante
la implementación de algoritmos de reemplazo de página.

---

## **Integrantes del Equipo**  

- **Brendan Rubilar Vivanco** - *2021750657*  
- **Tomás Cid Muñoz** - *2021454047*  
- **Luciano Argomedo** - *2021422935*  
- **Franchesca Mora Chehuan** - *2021424903*  

---
1. **circularBuffer.cpp**
- Ejemplo de compilación para circularBuffer.cpp: g++ circularBuffer.cpp -o simulapc
- Ejemplo de ejecución para circularBuffer.cpp: ./simulapc -p 10 -c 5 -s 50 -t 1
  
Este retorna un archivo log, con el proceso y actualizaciones del tamaño de la cola.

2. **simuladorMV.cpp**
- Ejemplo de compilación para simuladorMV.cpp: g++ simuladorMV.cpp -o mvirtual
- Ejemplo de ejecución para simuladorMV.cpp: ./mvirtual -m 3 -a FIFO -f archivo.txt
  
Se debe dar un archivo con un caso a evaluar, por ejemplo:

- "0 1 3 4 1 2 5 1 2 3 4"
- "0 1 2 3 4 0 1 2 3 4 5 6 7 8 9 5 6 7 8 9"

Este retorna en consola un mensaje con el número de fallos de página con respecto al número de marcos iniciales durante la ejecución.

