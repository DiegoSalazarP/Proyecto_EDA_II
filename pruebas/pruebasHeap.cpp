#include <iostream>
#include <cassert>
#include <chrono>
#include <cstdio>
#include "../modelos/Pedido.h"
#include "../estructuras/MinHeap.h"

int main() {
    std::cout << "=== INICIANDO PRUEBAS UNITARIAS DE MIN-HEAP ===\n\n";

    // Capacidad inicial pequeña (2) para probar también el redimensionamiento dinámico
    MinHeap heap(2);

    assert(heap.estaVacio() == true);
    assert(heap.obtenerMinimo() == nullptr);
    assert(heap.extraerMinimo() == nullptr);

    // 1 = SameDay, 2 = Express, 3 = Normal
    Pedido* pNormal   = new Pedido{"PD-000001", "11111111", "Normal",  3, 5000, "2026-11-10", "08:00:00", "Pendiente"};
    Pedido* pExpress  = new Pedido{"PD-000002", "22222222", "Express", 2, 8000, "2026-11-10", "09:00:00", "Pendiente"};
    Pedido* pSameDay2 = new Pedido{"PD-000003", "33333333", "SameDay", 1, 9500, "2026-11-10", "10:30:00", "Pendiente"};
    Pedido* pSameDay1 = new Pedido{"PD-000004", "44444444", "SameDay", 1, 7200, "2026-11-10", "09:15:00", "Pendiente"};

    // HEAP-01: Inserción y redimensionamiento automático
    assert(heap.insertar(pNormal) == true);
    assert(heap.insertar(pExpress) == true);
    assert(heap.insertar(pSameDay2) == true); // Dispara redimensionamiento (2 -> 4)
    assert(heap.insertar(pSameDay1) == true);
    assert(heap.obtenerCantidad() == 4);
    assert(heap.estaVacio() == false);
    std::cout << "[OK] HEAP-01: Insercion y redimensionamiento dinamico sin STL.\n";

    // HEAP-02: Consulta del mínimo O(1) con desempate por fecha/hora
    assert(heap.obtenerMinimo() == pSameDay1); // Prioridad 1 y hora 09:15:00 < 10:30:00
    std::cout << "[OK] HEAP-02: Consulta obtenerMinimo() O(1) con desempate cronologico.\n";

    // HEAP-03: Extracción ordenada por urgencia O(log n)
    assert(heap.extraerMinimo() == pSameDay1);
    assert(heap.extraerMinimo() == pSameDay2);
    assert(heap.extraerMinimo() == pExpress);
    assert(heap.extraerMinimo() == pNormal);
    assert(heap.estaVacio() == true);
    assert(heap.obtenerCantidad() == 0);
    std::cout << "[OK] HEAP-03: Extraccion estricta por prioridad (SameDay -> Express -> Normal).\n";

    // HEAP-04: Verificar que los objetos Pedido* siguen vivos en memoria
    assert(pSameDay1->idPedido == "PD-000004");
    assert(pNormal->idPedido == "PD-000001");
    std::cout << "[OK] HEAP-04: Destruccion/extraccion sin liberar Pedido* compartido.\n\n";

    delete pNormal;
    delete pExpress;
    delete pSameDay2;
    delete pSameDay1;

    // Prueba de rendimiento con 10,000 pedidos
    const int N = 10000;
    MinHeap heapMasivo(16);
    Pedido** pedidosPrueba = new Pedido*[N];

    for (int i = 0; i < N; ++i) {
        char bufferId[16];
        std::snprintf(bufferId, sizeof(bufferId), "PD-%06d", i + 1);
        int prioridad = (i % 3) + 1;
        pedidosPrueba[i] = new Pedido{
            bufferId, "45879123", "Normal", prioridad, 5000 + i, "2026-11-11", "12:00:00", "Pendiente"
        };
    }

    auto inicioInsercion = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < N; ++i) {
        heapMasivo.insertar(pedidosPrueba[i]);
    }
    auto finInsercion = std::chrono::high_resolution_clock::now();
    auto msInsercion = std::chrono::duration_cast<std::chrono::milliseconds>(finInsercion - inicioInsercion).count();

    auto inicioExtraccion = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < N; ++i) {
        heapMasivo.extraerMinimo();
    }
    auto finExtraccion = std::chrono::high_resolution_clock::now();
    auto msExtraccion = std::chrono::duration_cast<std::chrono::milliseconds>(finExtraccion - inicioExtraccion).count();

    std::cout << "=== RESULTADOS DE RENDIMIENTO MIN-HEAP (10,000 REGISTROS) ===\n";
    std::cout << "Tiempo insercion masiva O(n log n):  " << msInsercion << " ms\n";
    std::cout << "Tiempo extraccion total O(n log n):  " << msExtraccion << " ms\n";

    for (int i = 0; i < N; ++i) {
        delete pedidosPrueba[i];
    }
    delete[] pedidosPrueba;

    std::cout << "\nTODAS LAS PRUEBAS DE MIN-HEAP PASARON EXITOSAMENTE.\n";
    return 0;
}