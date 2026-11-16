#include <iostream>
#include <string>
#include <limits>
#include <cctype>   // Para std::isdigit
#include "servicios/GestorPedidos.h"
#include "servicios/LectorCSV.h"
#include "modelos/Pedido.h" // Requerido para acceder a los atributos del const Pedido*

// --------------------------------------------------
// FUNCIONES AUXILIARES DE CONSOLA Y PRESENTACIÓN
// --------------------------------------------------

// Limpia el buffer tras usar std::cin >> para evitar que std::getline capture un salto de línea vacío
void limpiarBuffer() {
    if (std::cin.fail()) {
        std::cin.clear();
    }
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

// Función de pausa segura (asume que el buffer ya está limpio antes de invocarla)
void pausarYLimpiar() {
    std::cout << "\nPresione ENTER para continuar...";
    std::string dummy;
    std::getline(std::cin, dummy);
    std::cout << "\n========================================\n\n";
}

bool parsearMonto(const std::string& str, long long& centavos) {
    if (str.empty()) return false;

    int puntos = 0;
    std::string enteros = "", decimales = "";

    for (char c : str) {
        if (c == '.') {
            puntos++;
        } else if (std::isdigit(static_cast<unsigned char>(c))) {
            if (puntos == 0) enteros += c;
            else decimales += c;
        } else {
            return false; // Rechaza negativos ('-') y caracteres no válidos
        }
    }

    if (puntos > 1 || decimales.length() > 2) return false; // Máximo 1 punto y 2 decimales

    if (enteros.empty()) enteros = "0";
    if (decimales.empty()) decimales = "00";
    else if (decimales.length() == 1) decimales += "0"; // Ejemplo: "5.5" -> "5.50"

    try {
        long long parteEntera = std::stoll(enteros);
        long long parteDecimal = std::stoll(decimales);

        // Prevención de desbordamiento antes de multiplicar por 100
        long long maxLL = std::numeric_limits<long long>::max();
        if (parteEntera > maxLL / 100) {
            return false;
        }

        long long enterosCentavos = parteEntera * 100;

        // Prevención de desbordamiento en la suma final
        if (maxLL - parteDecimal < enterosCentavos) {
            return false;
        }

        centavos = enterosCentavos + parteDecimal;
        return centavos > 0; // Rechaza montos de 0
    } catch (...) {
        return false;
    }
}

void mostrarMenu() {
    std::cout << "========================================\n";
    std::cout << "       SISTEMA LOGISTICO DE PEDIDOS     \n";
    std::cout << "========================================\n";
    std::cout << "1. Carga masiva de pedidos (CSV)\n";
    std::cout << "2. Registrar nuevo pedido manualmente\n";
    std::cout << "3. Buscar pedido por ID (Hash)\n";
    std::cout << "4. Despachar siguiente lote (Heap)\n";
    std::cout << "5. Consultar primeros 30 pedidos (AVL)\n";
    std::cout << "6. Auditoria por rango de fechas (AVL)\n";
    std::cout << "7. Actualizar estado de un pedido\n";
    std::cout << "8. Cancelar un pedido\n";
    std::cout << "9. Mostrar estadisticas generales\n";
    std::cout << "0. Salir del sistema\n";
    std::cout << "========================================\n";
    std::cout << "Ingrese una opcion: ";
}

// --------------------------------------------------
// CICLO PRINCIPAL
// --------------------------------------------------

int main() {
    // Constructor explícito:
    // - Hash: 20000 (garantiza factor de carga eficiente para 11000 elementos máx)
    // - Heap: 11000 (capacidad máxima estricta de la memoria central)
    GestorPedidos gestor(20000, 11000);

    int opcion = -1;

    do {
        mostrarMenu();

        // Blindaje contra caracteres inválidos en la selección del menú
        if (!(std::cin >> opcion)) {
            limpiarBuffer();
            std::cout << "\n[ERROR] Entrada no valida. Ingrese un numero.\n\n";
            continue;
        }

        // Consumir el '\n' remanente
        limpiarBuffer();

        switch (opcion) {
            case 1: {
                std::cout << "\n--- 1. CARGA MASIVA DE PEDIDOS ---\n";
                std::cout << "Ingrese la ruta del archivo (ej. pedidos_masivo.csv): ";
                std::string ruta;
                std::getline(std::cin, ruta);

                if (ruta.empty()) {
                    std::cout << "\n[ERROR] Debe ingresar una ruta valida.\n";
                    break;
                }

                int cargados = LectorCSV::cargarArchivo(ruta, gestor);
                if (cargados > 0) {
                    std::cout << "\n[!] Registros cargados exitosamente: " << cargados << "\n";
                } else {
                    std::cout << "\n[ERROR] No se cargaron registros. Verifique la ruta del archivo o el contenido.\n";
                }
                break;
            }
            case 2: {
                std::cout << "\n--- 2. REGISTRO MANUAL DE PEDIDO ---\n";

                std::string id, dni, tipo, fecha, hora, montoStr;
                int prioridad = 0;
                int opTipo = 0;
                long long montoCentavos = 0;

                std::cout << "ID del pedido (ej. PD-000001): ";
                std::getline(std::cin, id);

                std::cout << "DNI del cliente (8 digitos): ";
                std::getline(std::cin, dni);

                std::cout << "Tipo de envio (1: Normal, 2: Express, 3: SameDay): ";
                if (!(std::cin >> opTipo) || opTipo < 1 || opTipo > 3) {
                    limpiarBuffer();
                    std::cout << "\n[ERROR] Opcion invalida. Abortando registro.\n";
                    break;
                }
                limpiarBuffer();

                if (opTipo == 1) tipo = "Normal";
                else if (opTipo == 2) tipo = "Express";
                else tipo = "SameDay";

                std::cout << "Nivel de prioridad (1 al 10): ";
                if (!(std::cin >> prioridad) || prioridad < 1 || prioridad > 10) {
                    limpiarBuffer();
                    std::cout << "\n[ERROR] Prioridad invalida. Abortando registro.\n";
                    break;
                }
                limpiarBuffer();

                std::cout << "Monto en dolares (ej. 45.50): ";
                std::getline(std::cin, montoStr);

                if (!parsearMonto(montoStr, montoCentavos)) {
                    std::cout << "\n[ERROR] Monto invalido. Debe ser un numero positivo con maximo dos decimales. Abortando registro.\n";
                    break;
                }

                std::cout << "Fecha (YYYY-MM-DD): ";
                std::getline(std::cin, fecha);

                std::cout << "Hora (HH:MM:SS): ";
                std::getline(std::cin, hora);

                std::string estadoPorDefecto = "Pendiente";

                bool exito = gestor.registrarPedido(id, dni, tipo, prioridad, montoCentavos, fecha, hora, estadoPorDefecto);

                if (exito) {
                    std::cout << "\n[EXITO] Pedido " << id << " registrado correctamente en el sistema.\n";
                } else {
                    std::cout << "\n[ERROR] No se pudo registrar el pedido.\n";
                    std::cout << "Revise: ID duplicado, capacidad excedida o formato invalido de fecha/hora.\n";
                }
                break;
            }
            case 3: {
                std::cout << "\n--- 3. BUSCAR PEDIDO POR ID ---\n";
                std::cout << "Ingrese el ID del pedido (ej. PD-000001): ";
                std::string idBuscar;
                std::getline(std::cin, idBuscar);

                const Pedido* p = gestor.buscarPedido(idBuscar);

                if (p != nullptr) {
                    std::cout << "\n[!] Pedido encontrado:\n";
                    std::cout << "ID: " << p->idPedido
                              << " | DNI: " << p->dniCliente
                              << " | Tipo: " << p->tipoEnvio << "\n"
                              << "Prioridad: " << p->nivelPrioridad
                              << " | Monto: $" << (p->montoCentavos / 100) << "."
                              << (p->montoCentavos % 100 < 10 ? "0" : "") << (p->montoCentavos % 100) << "\n"
                              << "Fecha: " << p->fecha
                              << " | Hora: " << p->hora
                              << " | Estado: " << p->estado << "\n";
                } else {
                    std::cout << "\n[ERROR] El pedido con ID '" << idBuscar << "' no existe o fue cancelado.\n";
                }
                break;
            }
            case 4: {
                std::cout << "\n--- 4. DESPACHAR LOTE DE PEDIDOS ---\n";
                std::cout << "Ingrese la cantidad de pedidos a despachar: ";
                int n = 0;

                if (std::cin >> n && n > 0) {
                    limpiarBuffer();
                    // El contrato exige que el Gestor imprima directamente
                    gestor.despacharPedidos(n);
                } else {
                    limpiarBuffer();
                    std::cout << "\n[ERROR] Ingrese una cantidad entera mayor que cero.\n";
                }
                break;
            }
            case 5: {
                // El contrato exige que el AVL devuelva un arreglo y main.cpp se encargue de imprimirlo
                const Pedido* top30[30];
                int cantidadEncontrada = 0;

                gestor.obtenerTop30Cronologico(top30, cantidadEncontrada);

                std::cout << "\n--- 5. PRIMEROS 30 PEDIDOS (ORDEN CRONOLOGICO) ---\n";
                if (cantidadEncontrada == 0) {
                    std::cout << "No hay pedidos registrados o todos fueron cancelados.\n";
                } else {
                    for (int i = 0; i < cantidadEncontrada; ++i) {
                        const Pedido* p = top30[i];
                        std::cout << (i + 1) << ". " << p->idPedido
                                  << " | Fecha/Hora: " << p->fecha << " " << p->hora
                                  << " | Monto: $" << (p->montoCentavos / 100) << "."
                                  << (p->montoCentavos % 100 < 10 ? "0" : "") << (p->montoCentavos % 100)
                                  << " | Estado: " << p->estado << "\n";
                    }
                    std::cout << "\nPedidos mostrados: " << cantidadEncontrada << "\n";
                }
                break;
            }
            case 6: {
                std::cout << "\n--- 6. AUDITORIA POR RANGO DE FECHAS ---\n";
                std::string fechaInicio, fechaFin;

                std::cout << "Ingrese fecha de inicio (YYYY-MM-DD): ";
                std::getline(std::cin, fechaInicio);

                std::cout << "Ingrese fecha de fin (YYYY-MM-DD): ";
                std::getline(std::cin, fechaFin);

                int cantidad = 0;
                long long montoAcumulado = 0;

                bool exito = gestor.auditarPorRangoFecha(fechaInicio, fechaFin, cantidad, montoAcumulado);

                if (exito) {
                    std::cout << "\n[!] Auditoria Completada:\n";
                    std::cout << "Pedidos encontrados en el rango: " << cantidad << "\n";
                    std::cout << "Monto total del rango: $" << (montoAcumulado / 100) << "."
                              << (montoAcumulado % 100 < 10 ? "0" : "") << (montoAcumulado % 100) << "\n";
                } else {
                    std::cout << "\n[ERROR] Las fechas ingresadas son invalidas o el orden es incorrecto.\n";
                }
                break;
            }
            case 7: {
                std::cout << "\n--- 7. ACTUALIZAR ESTADO DE PEDIDO ---\n";
                std::cout << "ID del pedido: ";
                std::string idActualizar, nuevoEstado;
                std::getline(std::cin, idActualizar);

                // Considerando el diseño estricto de la máquina de estados, normalmente se transita a "Entregado"
                std::cout << "Nuevo estado (ej. Entregado): ";
                std::getline(std::cin, nuevoEstado);

                if (gestor.actualizarEstado(idActualizar, nuevoEstado)) {
                    std::cout << "\n[EXITO] El estado del pedido se actualizo a '" << nuevoEstado << "'.\n";
                } else {
                    std::cout << "\n[ERROR] No se pudo actualizar. Posibles causas:\n"
                              << "- El ID no existe o ya fue cancelado.\n"
                              << "- Transicion no permitida (solo se permite pasar a 'Entregado' si esta 'EnTransito').\n";
                }
                break;
            }
            case 8: {
                std::cout << "\n--- 8. CANCELAR PEDIDO ---\n";
                std::cout << "ID del pedido a cancelar: ";
                std::string idCancelar;
                std::getline(std::cin, idCancelar);

                if (gestor.cancelarPedido(idCancelar)) {
                    std::cout << "\n[EXITO] El pedido " << idCancelar << " fue cancelado correctamente.\n";
                } else {
                    std::cout << "\n[ERROR] No se pudo cancelar el pedido.\n"
                              << "Motivo: El ID no existe o el pedido no esta en estado 'Pendiente'.\n";
                }
                break;
            }
            case 9: {
                // Impresión O(1) manejada internamente por el gestor
                gestor.mostrarEstadisticas();
                break;
            }
            case 0: {
                std::cout << "\nSaliendo del sistema...\n";
                break;
            }
            default: {
                std::cout << "\n[ERROR] Opcion no valida. Intente nuevamente.\n";
                break;
            }
        }

        if (opcion != 0) {
            pausarYLimpiar();
        }

    } while (opcion != 0);

    return 0;
}
