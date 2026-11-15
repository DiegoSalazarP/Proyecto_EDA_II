#include "MinHeap.h"

MinHeap::MinHeap(int capacidadInicial) {
    this->capacidad = (capacidadInicial > 0) ? capacidadInicial : 10;
    this->cantidad = 0;
    this->arreglo = new Pedido*[this->capacidad];
}

MinHeap::~MinHeap() {
    // Libera el arreglo de punteros; GestorPedidos libera los objetos Pedido
    delete[] arreglo;
}

bool MinHeap::esMayorUrgencia(const Pedido* p1, const Pedido* p2) const {
    if (p1 == nullptr) return false;
    if (p2 == nullptr) return true;

    // 1. Menor número = mayor urgencia (ej. 1 es más urgente que 3)
    if (p1->nivelPrioridad != p2->nivelPrioridad) {
        return p1->nivelPrioridad < p2->nivelPrioridad;
    }
    // 2. Desempate por fecha (YYYY-MM-DD permite comparación lexicográfica directa)
    if (p1->fecha != p2->fecha) {
        return p1->fecha < p2->fecha;
    }
    // 3. Desempate por hora (HH:MM:SS)
    if (p1->hora != p2->hora) {
        return p1->hora < p2->hora;
    }
    // 4. Desempate final por idPedido
    return p1->idPedido < p2->idPedido;
}

void MinHeap::siftUp(int indice) {
    while (indice > 0) {
        int padre = (indice - 1) / 2;
        if (esMayorUrgencia(arreglo[indice], arreglo[padre])) {
            Pedido* temp = arreglo[indice];
            arreglo[indice] = arreglo[padre];
            arreglo[padre] = temp;
            indice = padre;
        } else {
            break;
        }
    }
}

void MinHeap::siftDown(int indice) {
    while (true) {
        int izq = 2 * indice + 1;
        int der = 2 * indice + 2;
        int masUrgente = indice;

        if (izq < cantidad && esMayorUrgencia(arreglo[izq], arreglo[masUrgente])) {
            masUrgente = izq;
        }
        if (der < cantidad && esMayorUrgencia(arreglo[der], arreglo[masUrgente])) {
            masUrgente = der;
        }

        if (masUrgente != indice) {
            Pedido* temp = arreglo[indice];
            arreglo[indice] = arreglo[masUrgente];
            arreglo[masUrgente] = temp;
            indice = masUrgente;
        } else {
            break;
        }
    }
}

bool MinHeap::insertar(Pedido* pedido) {
    if (pedido == nullptr) return false;

    // Redimensionar automáticamente si se alcanza la capacidad inicial
    if (cantidad == capacidad) {
        int nuevaCapacidad = capacidad * 2;
        Pedido** nuevoArreglo = new Pedido*[nuevaCapacidad];
        for (int i = 0; i < cantidad; i++) {
            nuevoArreglo[i] = arreglo[i];
        }
        delete[] arreglo;
        arreglo = nuevoArreglo;
        capacidad = nuevaCapacidad;
    }

    arreglo[cantidad] = pedido;
    siftUp(cantidad);
    cantidad++;
    return true;
}

Pedido* MinHeap::obtenerMinimo() const {
    if (estaVacio()) return nullptr;
    return arreglo[0];
}

Pedido* MinHeap::extraerMinimo() {
    if (estaVacio()) return nullptr;

    Pedido* minimo = arreglo[0];
    cantidad--;

    if (cantidad > 0) {
        arreglo[0] = arreglo[cantidad];
        siftDown(0);
    }
    arreglo[cantidad] = nullptr;

    return minimo;
}

bool MinHeap::estaVacio() const {
    return cantidad == 0;
}

int MinHeap::obtenerCantidad() const {
    return cantidad;
}