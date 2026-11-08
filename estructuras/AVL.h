#ifndef AVL_H
#define AVL_H

#include "../modelos/Pedido.h"
#include <string>

struct NodoAVL {
    Pedido* pedido;
    NodoAVL* izquierdo;
    NodoAVL* derecho;
    int altura;

    NodoAVL(Pedido* p) : pedido(p), izquierdo(nullptr), derecho(nullptr), altura(1) {}
};

class AVL {
private:
    NodoAVL* raiz;

    int obtenerAltura(NodoAVL* nodo);
    int obtenerBalance(NodoAVL* nodo);
    NodoAVL* rotacionDerecha(NodoAVL* y);
    NodoAVL* rotacionIzquierda(NodoAVL* x);

    NodoAVL* insertarRecursivo(NodoAVL* nodo, Pedido* pedido);
    NodoAVL* eliminarRecursivo(NodoAVL* nodo, Pedido* pedido);
    NodoAVL* encontrarMinimo(NodoAVL* nodo);

    void inordenRecursivo(NodoAVL* nodo, int& contador);
    void consultarRangoRecursivo(NodoAVL* nodo, const std::string& fechaInicio, const std::string& fechaFin, int& cantidad, long long& montoAcumulado);

    // Función auxiliar para comparar las claves compuestas
    int compararClaves(Pedido* p1, Pedido* p2);

public:
    AVL();
    ~AVL(); // Libera todos los NodoAVL, NO los objetos Pedido

    bool insertar(Pedido* pedido);
    bool eliminar(Pedido* pedido); // Se pasa el pedido para extraer sus claves (fecha, hora, id)
    void inorden(); // Limitado a 30 según documento
    void consultarRango(const std::string& fechaInicio, const std::string& fechaFin);
};

#endif // AVL_H
