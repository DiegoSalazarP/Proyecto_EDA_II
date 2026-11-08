#ifndef PEDIDO_H
#define PEDIDO_H
#include <string>

struct Pedido {
    std::string idPedido;
    std::string dniCliente;
    std::string tipoEnvio;
    int nivelPrioridad;
    long long montoCentavos; // Para evitar problemas de precisión en decimales
    std::string fecha;       // Formato ISO YYYY-MM-DD
    std::string hora;        // Formato HH:MM:SS
    std::string estado;      // Pendiente, EnTransito, Entregado, Cancelado
};


#endif // PEDIDO_H
