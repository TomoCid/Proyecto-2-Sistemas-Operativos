#include <iostream>
#include <thread>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <fstream>
#include <chrono>
#include <cstdlib>
#include <string>
#include <getopt.h>

using namespace std;

class BufferCircular{
private:
    int capacidad_max;
    int inicio, fin, total_elementos;
    vector<int> almacenamiento;
    ofstream archivo_log;
    mutex mtx1;
    condition_variable no_lleno, no_vacio;

    void expandir_buffer(){
        inicio = 0;
        fin = total_elementos;
        capacidad_max = capacidad_max * 2;
        almacenamiento.resize(capacidad_max);

        archivo_log << "Se duplicó la cola a: " << capacidad_max << "\n";
    }

    void reducir_buffer(){
        inicio = 0;
        fin = total_elementos;
        capacidad_max = capacidad_max / 2;
        almacenamiento.resize(capacidad_max);

        archivo_log << "Se redujo la cola a: " << capacidad_max << "\n";
    }

    bool puedeAgregar() const {
        return total_elementos < capacidad_max;
    }

    bool puedeExtraer() const {
        return total_elementos > 0;
    }

public:
    BufferCircular(int tam_inicial)
        : capacidad_max(tam_inicial), almacenamiento(tam_inicial), inicio(0), fin(0), total_elementos(0) {}

    void inicializarLog(const std::string& nombre_log) {
        archivo_log.open(nombre_log);
        if (archivo_log.is_open()) {
            archivo_log << "Se inicializó el registro de operaciones\n";
        } else {
            throw std::runtime_error("No se pudo abrir el archivo de log");
        }
    }


    void agregar(int elemento){
        unique_lock<mutex> lock(mtx1);
        no_lleno.wait(lock, [this] { 
            return puedeAgregar(); 
        });

        almacenamiento[fin] = elemento;
        fin = (fin + 1) % capacidad_max;

        total_elementos++;
        if (total_elementos == capacidad_max){
            expandir_buffer();
        }

        archivo_log << " Agregado: " << elemento << " - Total en buffer: " << total_elementos << " - Capacidad: " << capacidad_max << "\n";
        no_vacio.notify_one();
    }

    bool extraer(int &elemento, int tiempo_max) {
        unique_lock<mutex> lock(mtx1);
        if (!no_vacio.wait_for(lock, chrono::seconds(tiempo_max), [this] { return puedeExtraer();})){
            return false;
        }

        elemento = almacenamiento[inicio];
        inicio = (inicio + 1) % capacidad_max;

        total_elementos--;
        if (total_elementos < capacidad_max / 4 && capacidad_max > 1) {
            reducir_buffer();
        }

        archivo_log << " Extraído: " << elemento << " - Total en buffer: " << total_elementos << " - Capacidad: " << capacidad_max << "\n";
        no_lleno.notify_one();
        return true;
    }
};

void generar_elementos(BufferCircular &cola, int tiempo_espera, int cantidad) {
    for (int i = 0; i < cantidad; i++) {
        int valor = rand() % 100;
        cola.agregar(valor);
        this_thread::sleep_for(chrono::milliseconds(tiempo_espera));
    }
}

void consumir_elementos(BufferCircular &cola, int tiempo_max_espera) {
    int valor;
    while (cola.extraer(valor, tiempo_max_espera)) {
        this_thread::sleep_for(chrono::milliseconds(10));
    }
}

int main(int argc, char *argv[]) {
    int num_productores = 0, num_consumidores = 0, tam_inicial = 0, tiempo_espera = 0;

    // Manejo de opciones con getopt
    int opt;
    while ((opt = getopt(argc, argv, "p:c:s:t:")) != -1) {
        switch (opt) {
        case 'p':
            num_productores = stoi(optarg);
            break;
        case 'c':
            num_consumidores = stoi(optarg);
            break;
        case 's':
            tam_inicial = stoi(optarg);
            break;
        case 't':
            tiempo_espera = stoi(optarg);
            break;
        default:
            cerr << "Uso: " << argv[0] << " -p<num_productores> -c<num_consumidores> -s<tam_inicial> -t<tiempo_espera>\n";
            return 1;
        }
    }

    if (num_productores <= 0 || num_consumidores <= 0 || tam_inicial <= 0 || tiempo_espera <= 0) {
        cerr << "Error: todos los parámetros deben ser mayores a 0.\n";
        return 1;
    }

    BufferCircular buffer(tam_inicial);
    buffer.inicializarLog("log.txt");
    vector<thread> hilos_productores, hilos_consumidores;
    int num_items = 20;

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
