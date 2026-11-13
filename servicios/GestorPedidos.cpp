#include "GestorPedidos.h"
#include <iostream>
#include <iomanip>
#include <climits>
#include <stdexcept>

// --------------------------------------------------
// CONSTRUCTOR Y DESTRUCTOR
// --------------------------------------------------

GestorPedidos::GestorPedidos(int capacidadInicialHash, int capacidadInicialHeap)
    : tablaHash(nullptr), minHeap(nullptr), arbolAVL(nullptr), registroMemoria(nullptr) {

    try {
        tablaHash = new TablaHash(capacidadInicialHash);
        minHeap   = new MinHeap(capacidadInicialHeap);
        arbolAVL  = new AVL();

        capacidadMemoria = 11000;
        cantidadMemoria  = 0;
        registroMemoria  = new Pedido*[capacidadMemoria];
    } catch (...) {
        delete tablaHash;
        delete minHeap;
        delete arbolAVL;
        delete[] registroMemoria;
        throw;
    }

    totalPedidosRegistrados = 0;
    cantidadPendientes      = 0;
    montoAcumulado          = 0;
}

GestorPedidos::~GestorPedidos() {
    delete tablaHash;
    delete minHeap;
    delete arbolAVL;

    for (int i = 0; i < cantidadMemoria; ++i) {
        delete registroMemoria[i];
    }
    delete[] registroMemoria;
}

// --------------------------------------------------
// ADMINISTRACIÓN DE PROPIEDAD DE MEMORIA
// --------------------------------------------------

void GestorPedidos::retirarPedidoDeMemoria(Pedido* pedido) {
    if (pedido == nullptr) return;

    for (int i = 0; i < cantidadMemoria; ++i) {
        if (registroMemoria[i] == pedido) {
            registroMemoria[i] = registroMemoria[cantidadMemoria - 1];
            cantidadMemoria--;
            delete pedido;
            return;
        }
    }
}

// --------------------------------------------------
// INSERCIÓN TRANSACCIONAL ESTRICTA
// --------------------------------------------------

bool GestorPedidos::registrarPedido(const std::string& id, const std::string& dni, const std::string& tipo,
                                     int prioridad, long long montoCentavos, const std::string& fecha,
                                     const std::string& hora, const std::string& estado) {

    // 1. Validaciones de integridad y dominio (independientes de LectorCSV)
    if (cantidadMemoria >= capacidadMemoria) return false;
    if (id.empty()) return false;
    if (dni.length() != 8) return false; // Se asume validación de dígitos externa o básica aquí
    if (tipo != "Normal" && tipo != "Express" && tipo != "SameDay") return false;
    if (prioridad < 1 || prioridad > 10) return false;
    if (montoCentavos <= 0) return false;
    if (fecha.length() != 10 || hora.length() != 8) return false;
    if (estado != "Pendiente" && estado != "EnTransito" && estado != "Entregado") return false;

    // 2. Prevención de desbordamiento aritmético en el monto acumulado
    if (montoCentavos > LLONG_MAX - montoAcumulado) {
        return false;
    }

    if (tablaHash->buscar(id) != nullptr) return false;

    // 3. Creación del objeto (falla segura por bad_alloc)
    Pedido* nuevo = new Pedido();
    nuevo->idPedido       = id;
    nuevo->dniCliente     = dni;
    nuevo->tipoEnvio      = tipo;
    nuevo->nivelPrioridad = prioridad;
    nuevo->montoCentavos  = montoCentavos;
    nuevo->fecha          = fecha;
    nuevo->hora           = hora;
    nuevo->estado         = estado;

    registroMemoria[cantidadMemoria++] = nuevo;

    // 4. Inserción coordinada.
    // Se delega la garantía de excepción fuerte a las implementaciones de Hash, AVL y Heap.
    bool okHash = tablaHash->insertar(nuevo);
    bool okAVL  = false;
    bool okHeap = false;

    if (okHash) {
        okAVL = arbolAVL->insertar(nuevo);
        if (okAVL) {
            okHeap = minHeap->insertar(nuevo);
        }
    }

    // 5. Rollback determinista (sin ocultar excepciones con catch vacíos)
    if (!okHash || !okAVL || !okHeap) {
        if (okAVL)  arbolAVL->eliminar(nuevo);
        if (okHash) tablaHash->eliminar(id);

        cantidadMemoria--;
        delete nuevo;
        return false;
    }

    totalPedidosRegistrados++;
    montoAcumulado += montoCentavos;
    if (estado == "Pendiente") cantidadPendientes++;

    return true;
}

// --------------------------------------------------
// OPERACIONES DE NEGOCIO Y CANCELACIÓN
// --------------------------------------------------

void GestorPedidos::despacharPedidos(int n) {
    if (n <= 0) return;
    std::cout << "\n=== DESPACHO DE HASTA " << n << " PEDIDOS URGENTES ===\n";
    int despachados = 0;

    while (despachados < n && !minHeap->estaVacio()) {
        Pedido* masUrgente = minHeap->extraerMinimo();
        if (masUrgente == nullptr) continue;

        if (masUrgente->estado == "Cancelado") {
            retirarPedidoDeMemoria(masUrgente);
            continue;
        }

        if (masUrgente->estado == "Pendiente") {
            masUrgente->estado = "EnTransito";
            cantidadPendientes--;
            despachados++;
            std::cout << "Despachado [" << despachados << "/" << n << "]: "
                      << masUrgente->idPedido << " | Prioridad: " << masUrgente->nivelPrioridad
                      << " | Fecha/Hora: " << masUrgente->fecha << " " << masUrgente->hora << "\n";
        }
    }
    std::cout << "Pedidos efectivamente despachados: " << despachados << "\n";
}

bool GestorPedidos::cancelarPedido(std::string idPedido) {
    Pedido* p = tablaHash->buscar(idPedido);

    if (p == nullptr || p->estado != "Pendiente") {
        return false;
    }

    // Eliminación coordinada
    bool elimHash = tablaHash->eliminar(idPedido);
    if (!elimHash) {
        return false; // Si falla aquí, las estructuras siguen íntegras.
    }

    bool elimAVL = arbolAVL->eliminar(p);

    // Si AVL falla tras haber eliminado de Hash, se aplica compensación estricta
    if (!elimAVL) {
        bool recuperado = tablaHash->insertar(p);
        if (!recuperado) {
            // Falla catastrófica: el sistema quedó inconsistente y no pudo recuperarse.
            throw std::runtime_error("INCONSISTENCIA CRÍTICA: Falló la eliminación en AVL y la recuperación en Hash.");
        }
        return false;
    }

    p->estado = "Cancelado";
    totalPedidosRegistrados--;
    cantidadPendientes--;
    montoAcumulado -= p->montoCentavos;

    return true;
}

bool GestorPedidos::actualizarEstado(std::string idPedido, std::string nuevoEstado) {
    Pedido* p = tablaHash->buscar(idPedido);
    if (p == nullptr) return false;

    if (p->estado == "EnTransito" && nuevoEstado == "Entregado") {
        p->estado = "Entregado";
        return true;
    }
    if (p->estado == "Pendiente" && nuevoEstado == "Cancelado") {
        return cancelarPedido(idPedido);
    }
    return false;
}

// --------------------------------------------------
// ESTADÍSTICAS O(1)
// --------------------------------------------------

void GestorPedidos::mostrarEstadisticas() {
    std::cout << "\n=== ESTADISTICAS GENERALES DEL SISTEMA ===\n";
    std::cout << "Total de pedidos activos (no cancelados): " << totalPedidosRegistrados << "\n";
    std::cout << "Pedidos pendientes por despachar:         " << cantidadPendientes << "\n";

    std::cout << "Monto acumulado activo:                   $";
    unsigned long long valor = static_cast<unsigned long long>(montoAcumulado);
    std::cout << (valor / 100) << "."
              << std::setw(2) << std::setfill('0') << (valor % 100)
              << std::setfill(' ') << "\n";
    std::cout << "-------------------------------------------\n";
}
