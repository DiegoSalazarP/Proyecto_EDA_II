#include <iostream>
#include <cassert>
#include <chrono>
#include <cstdio>
#include "../modelos/Pedido.h"
#include "../estructuras/TablaHash.h"

int main() {
    std::cout << "=== INICIANDO PRUEBAS UNITARIAS DE TABLA HASH ===\n\n";

    // 1. Prueba básica: inserción, duplicados, búsqueda y eliminación sin borrar Pedido*
    TablaHash tabla(11); // Capacidad pequeña para forzar colisiones

    Pedido* p1 = new Pedido{"PD-000001", "12345678", "Express", 1, 4550, "2026-11-10", "08:30:00", "Pendiente"};
    Pedido* p2 = new Pedido{"PD-000002", "87654321", "Normal", 3, 12000, "2026-11-10", "09:15:00", "Pendiente"};
    Pedido* p3 = new Pedido{"PD-000003", "11223344", "SameDay", 1, 8990, "2026-11-10", "10:00:00", "Pendiente"};

    // HASH-01: Inserción
    assert(tabla.insertar(p1) == true);
    assert(tabla.insertar(p2) == true);
    assert(tabla.insertar(p3) == true);
    assert(tabla.obtenerCantidad() == 3);

    // Rechazo de duplicados
    assert(tabla.insertar(p1) == false);
    assert(tabla.obtenerCantidad() == 3);
    std::cout << "[OK] HASH-01: Insercion y rechazo de IDs duplicados.\n";

    // HASH-02 y HASH-03: Búsqueda con colisiones y búsqueda inexistente
    assert(tabla.buscar("PD-000001") == p1);
    assert(tabla.buscar("PD-000002") == p2);
    assert(tabla.buscar("PD-000003") == p3);
    assert(tabla.buscar("PD-999999") == nullptr);
    std::cout << "[OK] HASH-02 / HASH-03: Busqueda por ID y resolucion de colisiones.\n";

    // HASH-04: Eliminación de nodo sin destruir el objeto Pedido* compartido
    assert(tabla.eliminar("PD-000002") == true);
    assert(tabla.buscar("PD-000002") == nullptr);
    assert(tabla.obtenerCantidad() == 2);
    assert(p2->idPedido == "PD-000002"); // Sigue vivo en memoria (propiedad del gestor)
    assert(tabla.eliminar("PD-000002") == false);
    std::cout << "[OK] HASH-04: Eliminacion de nodo sin liberar Pedido* compartido.\n\n";

    delete p1;
    delete p2;
    delete p3;

    // 2. Prueba de rendimiento con 10,000 pedidos (para datos del informe IEEE)
    const int N = 10000;
    TablaHash tablaMasiva(10007);
    Pedido** pedidosPrueba = new Pedido*[N];

    for (int i = 0; i < N; ++i) {
        char bufferId[16];
        std::snprintf(bufferId, sizeof(bufferId), "PD-%06d", i + 1);
        pedidosPrueba[i] = new Pedido{
            bufferId, "45879123", "Normal", 2, 5000 + i, "2026-11-11", "12:00:00", "Pendiente"
        };
    }

    auto inicioCarga = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < N; ++i) {
        tablaMasiva.insertar(pedidosPrueba[i]);
    }
    auto finCarga = std::chrono::high_resolution_clock::now();
    auto msCarga = std::chrono::duration_cast<std::chrono::milliseconds>(finCarga - inicioCarga).count();

    auto inicioBusq = std::chrono::high_resolution_clock::now();
    Pedido* encontrado = tablaMasiva.buscar("PD-005000");
    auto finBusq = std::chrono::high_resolution_clock::now();
    auto nsBusq = std::chrono::duration_cast<std::chrono::nanoseconds>(finBusq - inicioBusq).count();

    assert(encontrado != nullptr && encontrado->idPedido == "PD-005000");

    std::cout << "=== RESULTADOS DE RENDIMIENTO (10,000 REGISTROS) ===\n";
    std::cout << "Pedidos en Tabla Hash: " << tablaMasiva.obtenerCantidad() << "\n";
    std::cout << "Tiempo carga masiva:   " << msCarga << " ms\n";
    std::cout << "Tiempo busqueda O(1):  " << nsBusq << " ns (" << nsBusq / 1000.0 << " us)\n";

    // Limpieza de memoria de prueba (simula GestorPedidos)
    for (int i = 0; i < N; ++i) {
        delete pedidosPrueba[i];
    }
    delete[] pedidosPrueba;

    std::cout << "\nTODAS LAS PRUEBAS PASARON EXITOSAMENTE.\n";
    return 0;
}