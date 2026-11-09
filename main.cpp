#include <iostream>
#include <string>
#include "modelos/Pedido.h"
#include "estructuras/AVL.h"

// Función auxiliar temporal para crear pedidos rápidamente en el main
Pedido* crearPedidoTemporal(std::string id, std::string fecha, std::string hora, std::string estado, long long monto) {
    Pedido* p = new Pedido();
    p->idPedido = id;
    p->fecha = fecha;
    p->hora = hora;
    p->estado = estado;
    p->montoCentavos = monto;
    return p;
}

int main() {
    std::cout << "=== INICIANDO PRUEBAS DEL ARBOL AVL ===\n\n";

    // 1. Instanciar el árbol
    AVL miArbol;

    // 2. Crear los pedidos dinámicamente (Simulando el GestorPedidos)
    // Usamos el formato YYYY-MM-DD y HH:MM para probar el orden lexicográfico
    Pedido* p1 = crearPedidoTemporal("P001", "2026-11-05", "14:30", "PENDIENTE", 15050); // $150.50
    Pedido* p2 = crearPedidoTemporal("P002", "2026-11-01", "09:15", "ENTREGADO", 2500);  // $25.00
    Pedido* p3 = crearPedidoTemporal("P003", "2026-11-08", "20:00", "EN RUTA", 30000);   // $300.00
    Pedido* p4 = crearPedidoTemporal("P004", "2026-11-05", "10:00", "CANCELADO", 550);   // $5.50
    Pedido* p5 = crearPedidoTemporal("P005", "2026-10-31", "23:59", "ENTREGADO", 8999);  // $89.99

    // 3. Prueba de Inserción
    std::cout << "[PRUEBA 1] Insertando pedidos...\n";
    miArbol.insertar(p1);
    miArbol.insertar(p2);
    miArbol.insertar(p3);
    miArbol.insertar(p4);
    miArbol.insertar(p5);

    // Prueba de duplicado (mismas claves: fecha, hora, idPedido)
    bool duplicadoRechazado = !miArbol.insertar(p1);
    std::cout << "Intento de insertar duplicado rechazado: "
              << (duplicadoRechazado ? "EXITO" : "FALLO") << "\n";

    // 4. Prueba de Recorrido (Debería imprimir ordenado cronológicamente)
    std::cout << "\n[PRUEBA 2] Recorrido Inorden (Orden Cronologico)\n";
    miArbol.inorden();

    // 5. Prueba de Consulta por Rango
    std::cout << "\n[PRUEBA 3] Consulta por Rango de Fechas\n";
    // Debería incluir p1, p2 y p4, ignorando p5 (octubre) y p3 (8 de nov)
    miArbol.consultarRango("2026-11-01", "2026-11-05");

    // 6. Prueba de Eliminación
    std::cout << "\n[PRUEBA 4] Eliminando pedido P004...\n";
    bool eliminado = miArbol.eliminar(p4);
    std::cout << "Eliminacion de P004: " << (eliminado ? "EXITO" : "FALLO") << "\n";

    std::cout << "Imprimiendo despues de eliminar:\n";
    miArbol.inorden();

    // 7. Limpieza de memoria manual
    // (El AVL destruye sus nodos, pero nosotros debemos destruir los Pedidos,
    // tal como lo hará tuGestorPedidos::~GestorPedidos())
    std::cout << "\n[LIMPIEZA] Liberando memoria de objetos Pedido...\n";
    delete p1;
    delete p2;
    delete p3;
    delete p4; // Aunque fue eliminado del árbol, el objeto sigue existiendo en memoria
    delete p5;

    std::cout << "Pruebas finalizadas con exito.\n";

    return 0;
}
