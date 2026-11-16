#include "AVL.h"
#include <iostream>
#include <iomanip>

// --------------------------------------------------
// FUNCIONES AUXILIARES
// --------------------------------------------------
namespace {

    // Libera exclusivamente los nodos del AVL.
    // No libera los objetos Pedido.
    void liberarNodos(NodoAVL* nodo) {

        if (nodo == nullptr) {
            return;
        }

        liberarNodos(nodo->izquierdo);
        liberarNodos(nodo->derecho);

        delete nodo;
    }

    // Muestra un monto almacenado en centavos.
    void imprimirMonto(long long montoCentavos) {
        bool negativo = montoCentavos < 0;

        //unsigned es un modificador que se usa para indicar que una variable solo almacenará números enteros positivos y el cero (sin signo negativo).
        unsigned long long valor = negativo
            ? static_cast<unsigned long long>(-(montoCentavos + 1)) + 1
            : static_cast<unsigned long long>(montoCentavos);

        if (negativo) {
            std::cout << "-";
        }

        std::cout << "$"
                  << valor / 100
                  << "."
                  << std::setw(2)
                  << std::setfill('0')
                  << valor % 100
                  << std::setfill(' ');
    }

    void imprimirPedido(const Pedido* pedido) { // <-- Se agregó const
        std::cout << "ID: " << pedido->idPedido
                  << " | Fecha: " << pedido->fecha
                  << " | Hora: " << pedido->hora
                  << " | Estado: " << pedido->estado
                  << " | Monto: ";

        imprimirMonto(pedido->montoCentavos);
        std::cout << "\n";
    }
}

// --------------------------------------------------
// CONSTRUCTOR Y DESTRUCTOR
// --------------------------------------------------
AVL::AVL() {
    raiz = nullptr;
}

AVL::~AVL() {
    liberarNodos(raiz);
    raiz = nullptr;
}

// --------------------------------------------------
// ALTURA Y FACTOR DE BALANCE
// --------------------------------------------------
int AVL::obtenerAltura(NodoAVL* nodo) {
    if (nodo == nullptr) {
        return 0;
    }
    return nodo->altura;
}

int AVL::obtenerBalance(NodoAVL* nodo) {
    if (nodo == nullptr) {
        return 0;
    }
    return obtenerAltura(nodo->izquierdo) - obtenerAltura(nodo->derecho);
}

// --------------------------------------------------
// ROTACIONES
// --------------------------------------------------
NodoAVL* AVL::rotacionDerecha(NodoAVL* y) {

    NodoAVL* x = y->izquierdo;
    NodoAVL* subArbol = x->derecho;

    // Realizar rotación
    x->derecho = y;
    y->izquierdo = subArbol;

    // Actualizar alturas
    int alturaIzq = obtenerAltura(y->izquierdo);
    int alturaDer = obtenerAltura(y->derecho);

    y->altura = 1 + (alturaIzq > alturaDer ? alturaIzq : alturaDer);

    alturaIzq = obtenerAltura(x->izquierdo);
    alturaDer = obtenerAltura(x->derecho);

    x->altura = 1 + (alturaIzq > alturaDer ? alturaIzq : alturaDer);

    return x;
}

NodoAVL* AVL::rotacionIzquierda(NodoAVL* x) {

    NodoAVL* y = x->derecho;
    NodoAVL* subArbol = y->izquierdo;

    // Realizar rotación
    y->izquierdo = x;
    x->derecho = subArbol;

    // Actualizar alturas
    int alturaIzq = obtenerAltura(x->izquierdo);
    int alturaDer = obtenerAltura(x->derecho);

    x->altura = 1 + (alturaIzq > alturaDer ? alturaIzq : alturaDer);

    alturaIzq = obtenerAltura(y->izquierdo);
    alturaDer = obtenerAltura(y->derecho);

    y->altura = 1 + (alturaIzq > alturaDer ? alturaIzq : alturaDer);

    return y;
}

// --------------------------------------------------
// COMPARACIÓN DE CLAVES COMPUESTAS
// --------------------------------------------------
int AVL::compararClaves(Pedido* p1, Pedido* p2) {

    if (p1->fecha < p2->fecha) return -1;
    if (p1->fecha > p2->fecha) return 1;

    if (p1->hora < p2->hora) return -1;
    if (p1->hora > p2->hora) return 1;

    if (p1->idPedido < p2->idPedido) return -1;
    if (p1->idPedido > p2->idPedido) return 1;

    return 0;
}

// --------------------------------------------------
// INSERCIÓN
// --------------------------------------------------
NodoAVL* AVL::insertarRecursivo(NodoAVL* nodo,Pedido* pedido) {
    // Encontramos la posición de inserción
    if (nodo == nullptr) {
        return new NodoAVL(pedido);
    }

    int comparacion = compararClaves(pedido, nodo->pedido);

    if (comparacion < 0) {
        nodo->izquierdo = insertarRecursivo(nodo->izquierdo, pedido);
    }

    else if (comparacion > 0) {
        nodo->derecho = insertarRecursivo(nodo->derecho, pedido);
    }

    else {
        // No se permiten claves compuestas duplicadas
        return nodo;
    }

    // Actualizar altura
    int alturaIzq = obtenerAltura(nodo->izquierdo);
    int alturaDer = obtenerAltura(nodo->derecho);

    nodo->altura = 1 + (alturaIzq > alturaDer ? alturaIzq : alturaDer);

    // Calcular balance
    int balance = obtenerBalance(nodo);

    // Caso LL
    if (balance > 1 && compararClaves(pedido, nodo->izquierdo->pedido) < 0) {
        return rotacionDerecha(nodo);
    }

    // Caso RR
    if (balance < -1 && compararClaves(pedido, nodo->derecho->pedido) > 0) {
        return rotacionIzquierda(nodo);
    }

    // Caso LR
    if (balance > 1 && compararClaves(pedido, nodo->izquierdo->pedido) > 0) {
        nodo->izquierdo = rotacionIzquierda(nodo->izquierdo);
        return rotacionDerecha(nodo);
    }

    // Caso RL
    if (balance < -1 && compararClaves(pedido, nodo->derecho->pedido) < 0) {
        nodo->derecho = rotacionDerecha(nodo->derecho);
        return rotacionIzquierda(nodo);
    }

    return nodo;
}

bool AVL::insertar(Pedido* pedido) {
    if (pedido == nullptr) {
        return false;
    }

    // Verificar que la clave no exista previamente
    NodoAVL* actual = raiz;

    while (actual != nullptr) {
        int comparacion = compararClaves(pedido, actual->pedido);

        if (comparacion == 0) {
            return false;
        }

        if (comparacion < 0) {
            actual = actual->izquierdo;
        }
        else {
            actual = actual->derecho;
        }
    }

    raiz = insertarRecursivo(raiz, pedido);

    return true;
}

// --------------------------------------------------
// BÚSQUEDA DEL MÍNIMO
// --------------------------------------------------
NodoAVL* AVL::encontrarMinimo(NodoAVL* nodo) {
    NodoAVL* actual = nodo;

    while (actual != nullptr && actual->izquierdo != nullptr) {
        actual = actual->izquierdo;
    }

    return actual;
}

// --------------------------------------------------
// ELIMINACIÓN
// --------------------------------------------------
NodoAVL* AVL::eliminarRecursivo(NodoAVL* nodo, Pedido* pedido) {
    if (nodo == nullptr) {
        return nodo;
    }

    int comparacion = compararClaves(pedido, nodo->pedido);

    if (comparacion < 0) {
        nodo->izquierdo = eliminarRecursivo(nodo->izquierdo, pedido);
    }

    else if (comparacion > 0) {
        nodo->derecho = eliminarRecursivo(nodo->derecho, pedido);
    }

    else {
        // Caso 1 y 2: cero o un hijo
        if (nodo->izquierdo == nullptr || nodo->derecho == nullptr) {
            NodoAVL* hijo = (nodo->izquierdo != nullptr) ? nodo->izquierdo : nodo->derecho;

            delete nodo;

            return hijo;
        }

        // Caso 3: dos hijos
        NodoAVL* sucesor = encontrarMinimo(nodo->derecho);

        Pedido* pedidoSucesor = sucesor->pedido;

        nodo->pedido = pedidoSucesor;

        nodo->derecho = eliminarRecursivo(nodo->derecho, pedidoSucesor);
    }

    // Actualizar altura
    int alturaIzq = obtenerAltura(nodo->izquierdo);
    int alturaDer = obtenerAltura(nodo->derecho);

    nodo->altura = 1 + (alturaIzq > alturaDer ? alturaIzq : alturaDer);

    // Calcular balance
    int balance = obtenerBalance(nodo);

    // Caso LL
    if (balance > 1 && obtenerBalance(nodo->izquierdo) >= 0) {
        return rotacionDerecha(nodo);
    }

    // Caso LR
    if (balance > 1 && obtenerBalance(nodo->izquierdo) < 0) {
        nodo->izquierdo = rotacionIzquierda(nodo->izquierdo);
        return rotacionDerecha(nodo);
    }

    // Caso RR
    if (balance < -1 && obtenerBalance(nodo->derecho) <= 0) {
        return rotacionIzquierda(nodo);
    }

    // Caso RL
    if (balance < -1 && obtenerBalance(nodo->derecho) > 0) {
        nodo->derecho = rotacionDerecha(nodo->derecho);
        return rotacionIzquierda(nodo);
    }
    return nodo;
}

bool AVL::eliminar(Pedido* pedido) {
    if (pedido == nullptr) {
        return false;
    }

    // Comprobar que la clave exista
    NodoAVL* actual = raiz;

    while (actual != nullptr) {
        int comparacion = compararClaves(pedido, actual->pedido);

        if (comparacion == 0) {
            raiz = eliminarRecursivo(raiz, pedido);
            return true;
        }

        if (comparacion < 0) {
            actual = actual->izquierdo;
        }

        else {
            actual = actual->derecho;
        }
    }
    return false;
}

// --------------------------------------------------
// RECORRIDO INORDEN (MÁXIMO 30 PEDIDOS)
// --------------------------------------------------
void AVL::inordenRecursivo(NodoAVL* nodo, const Pedido** resultados, int& contador) {
    if (nodo == nullptr || contador >= 30) {
        return;
    }

    inordenRecursivo(nodo->izquierdo, resultados, contador);

    if (contador < 30) {
        resultados[contador] = nodo->pedido;
        contador++;
    }

    inordenRecursivo(nodo->derecho, resultados, contador);
}

void AVL::obtenerTop30(const Pedido** resultados, int& contador) {
    contador = 0;
    if (resultados == nullptr) return; // Validación de seguridad
    inordenRecursivo(raiz, resultados, contador);
}

void AVL::inorden() {
    if (raiz == nullptr) {
        std::cout << "El arbol AVL esta vacio.\n";
        return;
    }

    // Usamos la misma lógica del gestor para no duplicar código
    const Pedido* top30[30];
    int contador = 0;

    obtenerTop30(top30, contador);

    std::cout << "\n--- PRIMEROS 30 PEDIDOS EN ORDEN CRONOLOGICO ---\n";
    for (int i = 0; i < contador; ++i) {
        imprimirPedido(top30[i]);
    }
    std::cout << "Pedidos mostrados: " << contador << "\n";
}

// --------------------------------------------------
// CONSULTA POR RANGO DE FECHAS
// --------------------------------------------------
void AVL::consultarRango(const std::string& fechaInicio, const std::string& fechaFin, int& cantidad, long long& montoAcumulado) {
    cantidad = 0;
    montoAcumulado = 0;
    if (fechaInicio > fechaFin) return;
    consultarRangoRecursivo(raiz, fechaInicio, fechaFin, cantidad, montoAcumulado);
}

void AVL::consultarRangoRecursivo(NodoAVL* nodo, const std::string& fechaInicio, const std::string& fechaFin, int& cantidad, long long& montoAcumulado) {
    if (nodo == nullptr) {
        return;
    }

    // PODA 1: Solo bajamos por la izquierda si la fecha del nodo actual es mayor o igual
    // a la de inicio. Si es estrictamente menor, todo a su izquierda también lo será.
    if (nodo->pedido->fecha >= fechaInicio) {
        consultarRangoRecursivo(nodo->izquierdo, fechaInicio, fechaFin, cantidad, montoAcumulado);
    }

    // Acumulación: Procesamos el nodo si está estrictamente dentro del intervalo.
    if (nodo->pedido->fecha >= fechaInicio && nodo->pedido->fecha <= fechaFin) {
        cantidad++;
        montoAcumulado += nodo->pedido->montoCentavos;
    }

    // PODA 2: Solo bajamos por la derecha si la fecha del nodo actual es menor o igual
    // a la de fin. Si es estrictamente mayor, todo a su derecha también lo será.
    if (nodo->pedido->fecha <= fechaFin) {
        consultarRangoRecursivo(nodo->derecho, fechaInicio, fechaFin, cantidad, montoAcumulado);
    }
}
