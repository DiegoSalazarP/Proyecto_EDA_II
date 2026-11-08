#ifndef GESTOR_PEDIDOS_H
#define GESTOR_PEDIDOS_H

#include "../modelos/Pedido.h"
#include "../estructuras/TablaHash.h"
#include "../estructuras/MinHeap.h"
#include "../estructuras/AVL.h"
#include <string>

class GestorPedidos {
private:
    TablaHash* tablaHash;
    MinHeap* minHeap;
    AVL* arbolAVL;

    // Administración centralizada de memoria
    Pedido** registroMemoria;
    int capacidadMemoria;
    int cantidadMemoria;

    void retirarPedidoDeMemoria(Pedido* pedido); // Elimina y libera

    // Estadísticas O(1) (Total activo excluye cancelados)
    int totalPedidosRegistrados;
    int cantidadPendientes;
    long long montoAcumulado;

public:
    GestorPedidos(int capacidadInicialHash, int capacidadInicialHeap);

    // Evitar copias accidentales para proteger la memoria
    GestorPedidos(const GestorPedidos&) = delete;
    GestorPedidos& operator=(const GestorPedidos&) = delete;

    ~GestorPedidos(); // 1. Destruye estructuras. 2. Libera objetos en registroMemoria. 3. Libera arreglo.

    // Transaccional: incluye el manejo seguro en registroMemoria
    bool registrarPedido(const std::string& id, const std::string& dni, const std::string& tipo,
                         int prioridad, long long montoCentavos, const std::string& fecha,
                         const std::string& hora, const std::string& estado);

    // Funciones de negocio
    void despacharPedidos(int n);
    bool cancelarPedido(std::string idPedido); // Actualiza estado, resta monto y elimina de Hash y AVL
    bool actualizarEstado(std::string idPedido, std::string nuevoEstado); // Ej: Pendiente -> Entregado

    // Estadísticas O(1)
    void mostrarEstadisticas();
};

#endif // GESTOR_PEDIDOS_H
