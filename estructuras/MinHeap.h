#ifndef MIN_HEAP_H
#define MIN_HEAP_H

#include "../modelos/Pedido.h"

class MinHeap {
private:
    Pedido** arreglo;
    int capacidad;
    int cantidad;

    void siftUp(int indice);
    void siftDown(int indice);

    // Función auxiliar para evaluar urgencia + desempate (fecha, hora, idPedido)
    bool esMayorUrgencia(const Pedido* p1,const Pedido* p2) const;

public:
    MinHeap(int capacidadInicial);
    ~MinHeap(); // Libera el arreglo dinámico, NO los objetos Pedido

    bool insertar(Pedido* pedido);
    Pedido* obtenerMinimo() const;
    Pedido* extraerMinimo();
    bool estaVacio() const;
    int obtenerCantidad() const; // Ojo: incluye pedidos despachados/cancelados no extraídos aún
};

#endif // MIN_HEAP_H
