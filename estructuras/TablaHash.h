#ifndef TABLA_HASH_H
#define TABLA_HASH_H

#include "../modelos/Pedido.h"
#include <string>

struct NodoHash {
    Pedido* pedido;
    NodoHash* siguiente;

    NodoHash(Pedido* p) : pedido(p), siguiente(nullptr) {}
};

class TablaHash {
private:
    NodoHash** buckets;
    int capacidad;
    int cantidad;

    int funcionHash(const std::string& idPedido);

public:
    TablaHash(int capacidadInicial);
    ~TablaHash(); // Libera los nodos NodoHash y el arreglo buckets, NO los objetos Pedido

    bool insertar(Pedido* pedido);
    Pedido* buscar(const std::string& idPedido);
    bool eliminar(const std::string& idPedido);
    int obtenerCantidad();
};

#endif // TABLA_HASH_H
