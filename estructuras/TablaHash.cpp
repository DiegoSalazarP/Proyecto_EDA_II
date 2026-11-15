#include "TablaHash.h"

// Constructor: inicializa el arreglo dinámico de buckets en nullptr
TablaHash::TablaHash(int capacidadInicial) {
    this->capacidad = (capacidadInicial > 0) ? capacidadInicial : 10007;
    this->cantidad = 0;
    this->buckets = new NodoHash*[this->capacidad];

    for (int i = 0; i < this->capacidad; ++i) {
        this->buckets[i] = nullptr;
    }
}

// Destructor: libera ÚNICAMENTE los NodoHash y el arreglo buckets (DT-05)
// NO libera los objetos Pedido* (responsabilidad de GestorPedidos - DT-04)
TablaHash::~TablaHash() {
    for (int i = 0; i < this->capacidad; ++i) {
        NodoHash* actual = this->buckets[i];
        while (actual != nullptr) {
            NodoHash* siguiente = actual->siguiente;
            delete actual;
            actual = siguiente;
        }
        this->buckets[i] = nullptr;
    }
    delete[] this->buckets;
    this->buckets = nullptr;
    this->cantidad = 0;
}

// Función de dispersión djb2
int TablaHash::funcionHash(const std::string& idPedido) {
    unsigned long hash = 5381;
    for (size_t i = 0; i < idPedido.length(); ++i) {
        hash = ((hash << 5) + hash) + static_cast<unsigned char>(idPedido[i]);
    }
    return static_cast<int>(hash % static_cast<unsigned long>(this->capacidad));
}

// Inserta un puntero a Pedido usando encadenamiento al inicio (O(1) promedio)
bool TablaHash::insertar(Pedido* pedido) {
    if (pedido == nullptr || pedido->idPedido.empty()) {
        return false;
    }

    // Rechazar identificadores duplicados
    if (buscar(pedido->idPedido) != nullptr) {
        return false;
    }

    int indice = funcionHash(pedido->idPedido);
    NodoHash* nuevoNodo = new NodoHash(pedido);
    nuevoNodo->siguiente = this->buckets[indice];
    this->buckets[indice] = nuevoNodo;
    this->cantidad++;

    return true;
}

// Busca un pedido por su idPedido (O(1) promedio)
Pedido* TablaHash::buscar(const std::string& idPedido) {
    if (idPedido.empty()) {
        return nullptr;
    }

    int indice = funcionHash(idPedido);
    NodoHash* actual = this->buckets[indice];

    while (actual != nullptr) {
        if (actual->pedido != nullptr && actual->pedido->idPedido == idPedido) {
            return actual->pedido;
        }
        actual = actual->siguiente;
    }

    return nullptr;
}

// Elimina el nodo de la tabla sin liberar el objeto Pedido* (O(1) promedio)
bool TablaHash::eliminar(const std::string& idPedido) {
    if (idPedido.empty()) {
        return false;
    }

    int indice = funcionHash(idPedido);
    NodoHash* actual = this->buckets[indice];
    NodoHash* anterior = nullptr;

    while (actual != nullptr) {
        if (actual->pedido != nullptr && actual->pedido->idPedido == idPedido) {
            if (anterior == nullptr) {
                this->buckets[indice] = actual->siguiente;
            } else {
                anterior->siguiente = actual->siguiente;
            }
            delete actual; // Solo se elimina el nodo Hash
            this->cantidad--;
            return true;
        }
        anterior = actual;
        actual = actual->siguiente;
    }

    return false;
}

// Retorna la cantidad total de pedidos almacenados
int TablaHash::obtenerCantidad() {
    return this->cantidad;
}