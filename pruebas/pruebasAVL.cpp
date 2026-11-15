#include <iostream>
#include <cassert>
#include <chrono>
#include <cstdio>
#include <string>
#include "../modelos/Pedido.h"
#include "../estructuras/AVL.h"

// Función auxiliar para facilitar la creación de pedidos
Pedido* crearP(std::string id, std::string fecha, std::string hora, long long monto) {
    Pedido* p = new Pedido();
    p->idPedido = id;
    p->fecha = fecha;
    p->hora = hora;
    p->montoCentavos = monto;
    p->estado = "Pendiente";
    return p;
}

int main() {
    std::cout << "=== INICIANDO PRUEBAS UNITARIAS DE ARBOL AVL ===\n\n";

    AVL arbol;

    // 1. Pruebas Funcionales Básicas
    Pedido* p1 = crearP("P1", "2026-10-01", "10:00:00", 1000); // $10.00
    Pedido* p2 = crearP("P2", "2026-10-02", "10:00:00", 2000); // $20.00
    Pedido* p3 = crearP("P3", "2026-10-03", "10:00:00", 3000); // $30.00
    Pedido* p4 = crearP("P4", "2026-10-04", "10:00:00", 4000); // $40.00
    Pedido* p5 = crearP("P5", "2026-10-05", "10:00:00", 5000); // $50.00

    // AVL-01: Inserción y balanceo (forzando rotaciones)
    arbol.insertar(p4);
    arbol.insertar(p2);
    arbol.insertar(p1);
    arbol.insertar(p3);
    arbol.insertar(p5);
    std::cout << "[OK] AVL-01: Insercion completada (balanceo interno ejecutado).\n";

    // AVL-02: Auditoría por rango
    int cantidad = 0;
    long long montoTotal = 0;
    arbol.consultarRango("2026-10-02", "2026-10-04", cantidad, montoTotal);
    assert(cantidad == 3);
    assert(montoTotal == 9000); // 2000 + 3000 + 4000
    std::cout << "[OK] AVL-02: Consulta por rango exacta y poda de ramas correcta.\n";

    // AVL-03: Eliminación
    arbol.eliminar(p2);
    cantidad = 0; montoTotal = 0;
    arbol.consultarRango("2026-10-02", "2026-10-04", cantidad, montoTotal);
    assert(cantidad == 2); // Ya no está p2
    assert(montoTotal == 7000); // 3000 + 4000
    std::cout << "[OK] AVL-03: Eliminacion de nodo sin romper el arbol.\n";

    // AVL-04: Propiedad de Memoria (Los pedidos siguen vivos)
    assert(p2->idPedido == "P2");
    std::cout << "[OK] AVL-04: Eliminacion de nodo no destruye el Pedido* compartido.\n\n";

    delete p1; delete p2; delete p3; delete p4; delete p5;

    // 2. Prueba de rendimiento con 10,000 pedidos
    const int N = 10000;
    AVL arbolMasivo;
    Pedido** pedidosPrueba = new Pedido*[N];

    for (int i = 0; i < N; ++i) {
        char bufferId[16];
        char bufferFecha[16];
        std::snprintf(bufferId, sizeof(bufferId), "PD-%06d", i + 1);
        // Distribuir fechas artificialmente para el rango
        int dia = (i % 28) + 1;
        int mes = (i % 12) + 1;
        std::snprintf(bufferFecha, sizeof(bufferFecha), "2026-%02d-%02d", mes, dia);

        pedidosPrueba[i] = crearP(bufferId, bufferFecha, "12:00:00", 5000 + i);
    }

    auto inicioInsercion = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < N; ++i) {
        arbolMasivo.insertar(pedidosPrueba[i]);
    }
    auto finInsercion = std::chrono::high_resolution_clock::now();
    auto msInsercion = std::chrono::duration_cast<std::chrono::milliseconds>(finInsercion - inicioInsercion).count();

    int cantRango = 0;
    long long montoRango = 0;
    auto inicioRango = std::chrono::high_resolution_clock::now();
    arbolMasivo.consultarRango("2026-05-01", "2026-08-31", cantRango, montoRango);
    auto finRango = std::chrono::high_resolution_clock::now();
    auto msRango = std::chrono::duration_cast<std::chrono::milliseconds>(finRango - inicioRango).count();

    std::cout << "=== RESULTADOS DE RENDIMIENTO AVL (10,000 REGISTROS) ===\n";
    std::cout << "Tiempo insercion masiva O(n log n): " << msInsercion << " ms\n";
    std::cout << "Tiempo consulta rango O(log n + K): " << msRango << " ms (Registros hallados: " << cantRango << ")\n";

    // Limpieza de memoria
    for (int i = 0; i < N; ++i) {
        delete pedidosPrueba[i];
    }
    delete[] pedidosPrueba;

    std::cout << "\nTODAS LAS PRUEBAS DE AVL PASARON EXITOSAMENTE.\n";
    return 0;
}
