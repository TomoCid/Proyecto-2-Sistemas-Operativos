#include <iostream>
#include <thread>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <fstream>
#include <chrono>
#include <cstdlib>
#include <string>

using namespace std;

class BufferCircular {
private:
    vector<int> almacenamiento;
    int capacidad_max;
    int inicio, fin, total_elementos;
    ofstream archivo_log;
    mutex mtx1;
    condition_variable no_lleno, no_vacio;


    void expandir_buffer() {
        capacidad_max *= 2;
        almacenamiento.resize(capacidad_max);
        inicio = 0;
        fin = total_elementos;
        archivo_log << "La cola ha sido duplicada a capacidad: " << capacidad_max << "\n";
    }

    void reducir_buffer() {
        capacidad_max /= 2;
        almacenamiento.resize(capacidad_max);
        inicio = 0;
        fin = total_elementos;
        archivo_log << "La cola ha sido reducida a capacidad: " << capacidad_max << "\n";
    }

public:
    BufferCircular(int tam_inicial, const string& nombre_log): 
    capacidad_max(tam_inicial), almacenamiento(tam_inicial), inicio(0), fin(0), total_elementos(0), archivo_log(nombre_log) {
        archivo_log << "Inicio del registro de operaciones\n";
    }

   void agregar(int elemento) {
    unique_lock<mutex> lock(mtx1);
    no_lleno.wait(lock, [this] { return total_elementos < capacidad_max; });

    // Asignamos el valor y actualizamos el índice de fin
    almacenamiento[fin] = elemento;
    fin = (fin + 1) % capacidad_max;
    total_elementos++;

    // Duplicar capacidad si está llena
    if (total_elementos == capacidad_max) {
        expandir_buffer();
    }

    archivo_log << "Agregado: " << elemento << " | Total en buffer: " << total_elementos << " | Capacidad: " << capacidad_max << "\n";
    no_vacio.notify_one();  // Notificamos a los consumidores de que hay un nuevo elemento.
}

bool extraer(int &elemento, int tiempo_max) {
    unique_lock<mutex> lock(mtx1);
    if (!no_vacio.wait_for(lock, chrono::seconds(tiempo_max), [this] { return total_elementos > 0; })) {
        return false;  // Retorna `false` si se agota el tiempo de espera y la cola está vacía.
    }

    // Extraemos el elemento y actualizamos el índice de inicio
    elemento = almacenamiento[inicio];
    inicio = (inicio + 1) % capacidad_max;
    total_elementos--;

    // Reducir capacidad si el buffer está infrautilizado
    if (total_elementos < capacidad_max / 4 && capacidad_max > 1) {
        reducir_buffer();
    }

    archivo_log << "Extraído: " << elemento << " | Total en buffer: " << total_elementos << " | Capacidad: " << capacidad_max << "\n";
    no_lleno.notify_one();  // Notificamos a los productores de que hay espacio disponible.
    return true;
}
};

void generar_elementos(BufferCircular &cola, int tiempo_espera, int cantidad) { //Productor
    for (int i = 0; i < cantidad; i++) {
        int valor = rand() % 100;
        cola.agregar(valor);
        this_thread::sleep_for(chrono::milliseconds(tiempo_espera));
    }
}

void consumir_elementos(BufferCircular &cola, int tiempo_max_espera) { //Consumidor
    int valor;
    while (cola.extraer(valor, tiempo_max_espera))
        this_thread::sleep_for(chrono::milliseconds(10));
}

int main(int argc, char *argv[]) {
    if (argc != 5) {
        cerr << "Uso: " << argv[0] << " Se esperan los argumentos: <num_productores> <num_consumidores> <tam_inicial> <tiempo_espera>\n";
        return 1;
    }
    
    int num_productores, num_consumidores, tam_inicial, tiempo_espera;
    num_productores = atoi(argv[1]);
    num_consumidores = atoi(argv[2]);
    tam_inicial = atoi(argv[3]);
    tiempo_espera = atoi(argv[4]);


    BufferCircular buffer(tam_inicial, "registro.txt");
    vector<thread> hilos_productores, hilos_consumidores;

    int num_items = 10;

    for (int i = 0; i < num_productores; i++) {
        hilos_productores.emplace_back(generar_elementos, ref(buffer), tiempo_espera, num_items);
    }

    for (int i = 0; i < num_consumidores; i++) {
        hilos_consumidores.emplace_back(consumir_elementos, ref(buffer), tiempo_espera);
    }

    for (auto &hilo : hilos_productores) if (hilo.joinable()) hilo.join();
    for (auto &hilo : hilos_consumidores) if (hilo.joinable()) hilo.join();
    
    return 0;
}
