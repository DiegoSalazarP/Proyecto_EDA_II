#include <iostream>
#include <chrono>
#include <string>
#include <iomanip>

#include "servicios/GestorPedidos.h"
#include "servicios/LectorCSV.h"
#include "modelos/Pedido.h"

using namespace std;
using namespace chrono;

// ============================================================
// CONFIGURACIÓN DEL BENCHMARK
// ============================================================

const int TOTAL_REGISTROS_CSV = 10000;
const int CAPACIDAD_HASH = 20000;
const int CAPACIDAD_HEAP = 11000;

const int REP_BUSQUEDAS = 10000;
const int REP_INSERCIONES = 1000;

const int PEDIDOS_PENDIENTES = 600;
const int PEDIDOS_EN_TRANSITO = 400;

const int REP_DESPACHOS = 100;
const int REP_ACTUALIZACIONES = 400;
const int REP_CANCELACIONES = 500;

// ============================================================
// FUNCIÓN AUXILIAR: PRESENTACIÓN DE RESULTADOS
// ============================================================

void imprimirFila(
    const string& operacion,
    long long tiempoTotalNs,
    int repeticiones,
    const string& nota = ""
) {
    double promedioNs = 0.0;

    if (repeticiones > 0) {
        promedioNs = static_cast<double>(tiempoTotalNs) / repeticiones;
    }

    cout << left << setw(35) << operacion
         << right << setw(15) << tiempoTotalNs
         << right << setw(18) << fixed << setprecision(2) << promedioNs
         << "  " << nota << "\n";
}

// ============================================================
// PROGRAMA PRINCIPAL
// ============================================================

int main() {

    cout << "\n";
    cout << "========================================================================================\n";
    cout << "                         BENCHMARK DE RENDIMIENTO - RELEASE                            \n";
    cout << "========================================================================================\n";

    cout << left << setw(35) << "OPERACION"
         << right << setw(15) << "TIEMPO TOTAL"
         << right << setw(18) << "PROMEDIO/OP"
         << "  NOTAS\n";

    cout << "----------------------------------------------------------------------------------------\n";

    // El registro central tiene una capacidad máxima de 11 000 pedidos.
    GestorPedidos gestor(CAPACIDAD_HASH, CAPACIDAD_HEAP);

    const string rutaCSV = "datos/pedidos_masivo.csv";

    // ========================================================
    // 1. CARGA MASIVA DE 10 000 REGISTROS
    // ========================================================

    auto inicioCarga = steady_clock::now();

    int cargados = LectorCSV::cargarArchivo(rutaCSV, gestor);

    auto finCarga = steady_clock::now();

    long long nsCarga =
        duration_cast<nanoseconds>(finCarga - inicioCarga).count();

    imprimirFila(
        "1. Carga masiva",
        nsCarga,
        1,
        "Registros cargados: " + to_string(cargados)
    );

    // Verificar que se hayan cargado exactamente 10 000 registros.
    if (cargados != TOTAL_REGISTROS_CSV) {
        cerr << "\n[ERROR] Se esperaban "
             << TOTAL_REGISTROS_CSV
             << " registros validos, pero se cargaron "
             << cargados << ".\n";

        return 1;
    }

    // ========================================================
    // 2. BÚSQUEDA POR ID (TABLA HASH)
    // ========================================================

    const string idPrueba = "PD-005000";

    // Verificar previamente que el ID exista.
    if (gestor.buscarPedido(idPrueba) == nullptr) {
        cerr << "\n[ERROR] El ID de prueba "
             << idPrueba
             << " no existe en el CSV.\n";

        return 1;
    }

    int encontrados = 0;

    auto inicioBusq = steady_clock::now();

    for (int i = 0; i < REP_BUSQUEDAS; i++) {

        const Pedido* encontrado = gestor.buscarPedido(idPrueba);

        if (encontrado != nullptr) {
            encontrados++;
        }
    }

    auto finBusq = steady_clock::now();

    long long nsBusq =
        duration_cast<nanoseconds>(finBusq - inicioBusq).count();

    if (encontrados != REP_BUSQUEDAS) {
        cerr << "\n[ERROR] Algunas busquedas no encontraron el pedido.\n";
        return 1;
    }

    imprimirFila(
        "2. Busqueda por ID (Hash)",
        nsBusq,
        REP_BUSQUEDAS,
        "Busquedas exitosas: " + to_string(encontrados)
    );

    // ========================================================
    // 3. CONSULTA CRONOLÓGICA TOP 30 (AVL)
    // ========================================================

    const Pedido* top30[30];
    int cantidadTop = 0;

    auto inicioTop = steady_clock::now();

    gestor.obtenerTop30Cronologico(top30, cantidadTop);

    auto finTop = steady_clock::now();

    long long nsTop =
        duration_cast<nanoseconds>(finTop - inicioTop).count();

    if (cantidadTop != 30) {
        cerr << "\n[ERROR] La consulta cronologica no devolvio 30 pedidos.\n";
        return 1;
    }

    imprimirFila(
        "3. Consulta cronologica Top 30",
        nsTop,
        1,
        "Resultados: " + to_string(cantidadTop)
    );

    // ========================================================
    // 4. CONSULTA POR RANGO DE FECHAS (AVL)
    // ========================================================

    int cantidadRango = 0;
    long long montoRango = 0;

    auto inicioRango = steady_clock::now();

    bool rangoCorrecto = gestor.auditarPorRangoFecha(
        "2026-01-01",
        "2026-12-31",
        cantidadRango,
        montoRango
    );

    auto finRango = steady_clock::now();

    long long nsRango =
        duration_cast<nanoseconds>(finRango - inicioRango).count();

    if (!rangoCorrecto) {
        cerr << "\n[ERROR] No se pudo ejecutar la consulta por rango.\n";
        return 1;
    }

    imprimirFila(
        "4. Consulta por rango de fechas",
        nsRango,
        1,
        "Registros encontrados: " + to_string(cantidadRango)
    );

    // ========================================================
    // PREPARACIÓN DE PEDIDOS AUXILIARES
    // ========================================================

    // Se preparan los identificadores antes de iniciar los cronómetros.
    // Así no se mide el costo de construir las cadenas.

    string idsPendientes[PEDIDOS_PENDIENTES];
    string idsEnTransito[PEDIDOS_EN_TRANSITO];

    for (int i = 0; i < PEDIDOS_PENDIENTES; i++) {
        idsPendientes[i] = "BENCH-P-" + to_string(i + 1);
    }

    for (int i = 0; i < PEDIDOS_EN_TRANSITO; i++) {
        idsEnTransito[i] = "BENCH-T-" + to_string(i + 1);
    }

    // ========================================================
    // 5. INSERCIÓN INDIVIDUAL DE 1 000 PEDIDOS
    // ========================================================

    int insercionesExitosas = 0;

    auto inicioIns = steady_clock::now();

    // Primer bloque: 600 pedidos Pendientes.
    for (int i = 0; i < PEDIDOS_PENDIENTES; i++) {

        bool registrado = gestor.registrarPedido(
            idsPendientes[i],
            "12345678",
            "Normal",
            10,
            2500,
            "2026-12-03",
            "15:00:00",
            "Pendiente"
        );

        if (registrado) {
            insercionesExitosas++;
        }
    }

    // Segundo bloque: 400 pedidos EnTransito.
    for (int i = 0; i < PEDIDOS_EN_TRANSITO; i++) {

        bool registrado = gestor.registrarPedido(
            idsEnTransito[i],
            "87654321",
            "Express",
            10,
            5000,
            "2026-12-03",
            "15:00:00",
            "EnTransito"
        );

        if (registrado) {
            insercionesExitosas++;
        }
    }

    auto finIns = steady_clock::now();

    long long nsIns =
        duration_cast<nanoseconds>(finIns - inicioIns).count();

    if (insercionesExitosas != REP_INSERCIONES) {
        cerr << "\n[ERROR] No se pudieron registrar los 1 000 pedidos auxiliares.\n";
        cerr << "Inserciones exitosas: " << insercionesExitosas << "\n";
        return 1;
    }

    imprimirFila(
        "5. Insercion individual",
        nsIns,
        REP_INSERCIONES,
        "Inserciones exitosas: " + to_string(insercionesExitosas)
    );

    // ========================================================
    // 6. DESPACHO DE 100 PEDIDOS URGENTES (MIN-HEAP)
    // ========================================================

    // Comprobar que los 600 pedidos auxiliares siguen pendientes.
    int pendientesDisponibles = 0;

    for (int i = 0; i < PEDIDOS_PENDIENTES; i++) {

        const Pedido* pedido = gestor.buscarPedido(idsPendientes[i]);

        if (pedido != nullptr && pedido->estado == "Pendiente") {
            pendientesDisponibles++;
        }
    }

    if (pendientesDisponibles < REP_DESPACHOS) {
        cerr << "\n[ERROR] No existen suficientes pedidos pendientes para el despacho.\n";
        return 1;
    }

    auto inicioDesp = steady_clock::now();

    gestor.despacharPedidos(REP_DESPACHOS);

    auto finDesp = steady_clock::now();

    long long nsDesp =
        duration_cast<nanoseconds>(finDesp - inicioDesp).count();

    imprimirFila(
        "6. Despacho de pedidos urgentes",
        nsDesp,
        REP_DESPACHOS,
        "Despachos solicitados: 100"
    );

    // ========================================================
    // 7. ACTUALIZACIÓN DE ESTADO
    // EN TRANSITO -> ENTREGADO
    // ========================================================

    int actualizacionesExitosas = 0;

    auto inicioUpd = steady_clock::now();

    for (int i = 0; i < PEDIDOS_EN_TRANSITO; i++) {

        bool actualizado = gestor.actualizarEstado(
            idsEnTransito[i],
            "Entregado"
        );

        if (actualizado) {
            actualizacionesExitosas++;
        }
    }

    auto finUpd = steady_clock::now();

    long long nsUpd =
        duration_cast<nanoseconds>(finUpd - inicioUpd).count();

    if (actualizacionesExitosas != REP_ACTUALIZACIONES) {
        cerr << "\n[ERROR] No se pudieron actualizar los 400 pedidos.\n";
        cerr << "Actualizaciones exitosas: "
             << actualizacionesExitosas << "\n";

        return 1;
    }

    imprimirFila(
        "7. Actualizacion de estado",
        nsUpd,
        REP_ACTUALIZACIONES,
        "EnTransito -> Entregado"
    );

    // ========================================================
    // 8. CANCELACIÓN DE 500 PEDIDOS
    // ========================================================

    // Se seleccionan 500 pedidos que continúen Pendientes
    // después de ejecutar el despacho.

    string idsCancelar[REP_CANCELACIONES];

    int cantidadCancelables = 0;

    for (int i = 0;
         i < PEDIDOS_PENDIENTES && cantidadCancelables < REP_CANCELACIONES;
         i++) {

        const Pedido* pedido = gestor.buscarPedido(idsPendientes[i]);

        if (pedido != nullptr && pedido->estado == "Pendiente") {

            idsCancelar[cantidadCancelables] = idsPendientes[i];

            cantidadCancelables++;
        }
    }

    if (cantidadCancelables != REP_CANCELACIONES) {
        cerr << "\n[ERROR] No se encontraron 500 pedidos pendientes para cancelar.\n";
        cerr << "Pedidos disponibles: " << cantidadCancelables << "\n";

        return 1;
    }

    int cancelacionesExitosas = 0;

    auto inicioDel = steady_clock::now();

    for (int i = 0; i < REP_CANCELACIONES; i++) {

        bool cancelado = gestor.cancelarPedido(idsCancelar[i]);

        if (cancelado) {
            cancelacionesExitosas++;
        }
    }

    auto finDel = steady_clock::now();

    long long nsDel =
        duration_cast<nanoseconds>(finDel - inicioDel).count();

    if (cancelacionesExitosas != REP_CANCELACIONES) {
        cerr << "\n[ERROR] No se pudieron cancelar los 500 pedidos.\n";
        cerr << "Cancelaciones exitosas: "
             << cancelacionesExitosas << "\n";

        return 1;
    }

    imprimirFila(
        "8. Cancelacion de pedidos",
        nsDel,
        REP_CANCELACIONES,
        "Eliminados de Hash y AVL"
    );

    // ========================================================
    // FINALIZACIÓN
    // ========================================================

    cout << "----------------------------------------------------------------------------------------\n";
    cout << "BENCHMARK FINALIZADO CORRECTAMENTE.\n";
    cout << "Registros iniciales:       " << cargados << "\n";
    cout << "Inserciones auxiliares:   " << insercionesExitosas << "\n";
    cout << "Actualizaciones exitosas: " << actualizacionesExitosas << "\n";
    cout << "Cancelaciones exitosas:   " << cancelacionesExitosas << "\n";
    cout << "========================================================================================\n";

    return 0;
}

