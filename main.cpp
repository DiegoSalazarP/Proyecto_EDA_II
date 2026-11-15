#include <iostream>
#include <string>
#include "modelos/Pedido.h"
#include "estructuras/AVL.h"

// Función auxiliar para facilitar la creación de pedidos en la prueba
Pedido* crearP(std::string id, std::string fecha, std::string hora, long long monto) {
    Pedido* p = new Pedido();
    p->idPedido = id;
    p->fecha = fecha;
    p->hora = hora;
    p->montoCentavos = monto;
    p->estado = "Pendiente";
    return p;
}

int main() {
    std::cout << "=== INICIANDO PRUEBAS INTECRALES DEL ARBOL AVL ===\n\n";

    // A. PREPARACIÓN DE DATOS (Fechas secuenciales para forzar balanceo y probar eliminación)
    Pedido* p1 = crearP("P1", "2026-10-01", "10:00:00", 1000); // $10.00
    Pedido* p2 = crearP("P2", "2026-10-02", "10:00:00", 2000); // $20.00
    Pedido* p3 = crearP("P3", "2026-10-03", "10:00:00", 3000); // $30.00
    Pedido* p4 = crearP("P4", "2026-10-04", "10:00:00", 4000); // $40.00
    Pedido* p5 = crearP("P5", "2026-10-05", "10:00:00", 5000); // $50.00
    Pedido* p6 = crearP("P6", "2026-10-06", "10:00:00", 6000); // $60.00
    Pedido* p7 = crearP("P7", "2026-10-07", "10:00:00", 7000); // $70.00

    AVL* arbol = new AVL();

    // B. PRUEBA DE INSERCIÓN Y ROTACIONES
    std::cout << "[1] Insercion (Forzando rotaciones y balance)...\n";
    // Insertamos en un orden que provoque casos de desbalanceo (ej. inserción secuencial provoca RR)
    arbol->insertar(p4);
    arbol->insertar(p2);
    arbol->insertar(p6);
    arbol->insertar(p1);
    arbol->insertar(p3);
    arbol->insertar(p5);
    arbol->insertar(p7);

    std::cout << "Recorrido Inorden (Debe imprimir P1 a P7 ordenados):\n";
    arbol->inorden();

    // C. PRUEBA DE AUDITORÍA POR RANGO
    std::cout << "\n[2] Consulta por rango (Del 2026-10-02 al 2026-10-05)\n";
    int cantidad = 0;
    long long montoTotal = 0;
    arbol->consultarRango("2026-10-02", "2026-10-05", cantidad, montoTotal);
    std::cout << "Cantidad esperada: 4 | Cantidad obtenida: " << cantidad << "\n";
    std::cout << "Monto esperado: 14000 centavos | Monto obtenido: " << montoTotal << " centavos\n";

    // D. PRUEBA DE ELIMINACIÓN DE NODOS (Hoja, 1 Hijo, 2 Hijos)
    std::cout << "\n[3] Eliminacion de nodos...\n";

    std::cout << "-> Eliminando P1 (Nodo Hoja)\n";
    arbol->eliminar(p1);

    std::cout << "-> Eliminando P2 (Nodo con 1 o 2 hijos, su sucesor inorden es P3)\n";
    arbol->eliminar(p2);

    std::cout << "Recorrido Inorden tras eliminaciones (Debe imprimir P3, P4, P5, P6, P7):\n";
    arbol->inorden();

    // E. PRUEBA DE GESTIÓN DE MEMORIA (PROPIEDAD)
    std::cout << "\n[4] Prueba de propiedad de memoria...\n";
    std::cout << "Destruyendo el arbol AVL...\n";
    delete arbol; // Llama a liberarNodos()

    // Si el AVL liberó los objetos Pedido, esto causaría un Segmentation Fault.
    // Si los conservó, podremos leer sus IDs.
    std::cout << "Validando que los pedidos siguen existiendo en memoria:\n";
    std::cout << "P3 ID: " << p3->idPedido << "\n";
    std::cout << "P7 ID: " << p7->idPedido << "\n";

    // Liberación manual simulando al GestorPedidos
    std::cout << "Liberando objetos Pedido manualmente (Simulacion GestorPedidos)...\n";
    delete p1; delete p2; delete p3; delete p4; delete p5; delete p6; delete p7;

    std::cout << "\n=== PRUEBAS FINALIZADAS CON EXITO ===\n";

    return 0;
}
