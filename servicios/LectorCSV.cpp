#include "LectorCSV.h"
#include <fstream>
#include <iostream>
#include <climits>

// --------------------------------------------------
// AUXILIARES DE CADENAS Y NÚMEROS
// --------------------------------------------------

std::string LectorCSV::trim(const std::string& str) {
    size_t primero = str.find_first_not_of(" \t\r\n");
    if (primero == std::string::npos) return "";
    size_t ultimo = str.find_last_not_of(" \t\r\n");
    return str.substr(primero, (ultimo - primero + 1));
}

bool LectorCSV::esNumeroEntero(const std::string& str) {
    if (str.empty()) return false;
    for (size_t i = 0; i < str.length(); ++i) {
        if (str[i] < '0' || str[i] > '9') return false;
    }
    return true;
}

// --------------------------------------------------
// VALIDACIÓN DE CALENDARIO Y HORA
// --------------------------------------------------

bool LectorCSV::esBisiesto(int anio) {
    return (anio % 4 == 0 && anio % 100 != 0) || (anio % 400 == 0);
}

int LectorCSV::diasEnMes(int mes, int anio) {
    if (mes == 2) {
        return esBisiesto(anio) ? 29 : 28;
    }
    if (mes == 4 || mes == 6 || mes == 9 || mes == 11) {
        return 30;
    }
    return 31;
}

bool LectorCSV::esFechaValida(const std::string& fecha) {
    if (fecha.length() != 10) return false;
    if (fecha[4] != '-' || fecha[7] != '-') return false;

    for (int i = 0; i < 10; ++i) {
        if (i == 4 || i == 7) continue;
        if (fecha[i] < '0' || fecha[i] > '9') return false;
    }

    int anio = std::stoi(fecha.substr(0, 4));
    int mes  = std::stoi(fecha.substr(5, 2));
    int dia  = std::stoi(fecha.substr(8, 2));

    if (anio <= 2000 || mes < 1 || mes > 12) return false;

    int maxDias = diasEnMes(mes, anio);
    return (dia >= 1 && dia <= maxDias);
}

bool LectorCSV::esHoraValida(const std::string& hora) {
    if (hora.length() != 8) return false;
    if (hora[2] != ':' || hora[5] != ':') return false;

    for (int i = 0; i < 8; ++i) {
        if (i == 2 || i == 5) continue;
        if (hora[i] < '0' || hora[i] > '9') return false;
    }

    int h = std::stoi(hora.substr(0, 2));
    int m = std::stoi(hora.substr(3, 2));
    int s = std::stoi(hora.substr(6, 2));

    return (h >= 0 && h <= 23 && m >= 0 && m <= 59 && s >= 0 && s <= 59);
}

// --------------------------------------------------
// VALIDACIÓN DE CAMPOS Y CONVERSIONES SEGURAS
// --------------------------------------------------

bool LectorCSV::esDniValido(const std::string& dni) {
    return (dni.length() == 8 && esNumeroEntero(dni));
}

bool LectorCSV::esTipoEnvioValido(const std::string& tipo) {
    return (tipo == "Normal" || tipo == "Express" || tipo == "SameDay");
}

bool LectorCSV::esEstadoValido(const std::string& estado) {
    return (estado == "Pendiente" ||
            estado == "EnTransito" ||
            estado == "Entregado");
}

bool LectorCSV::convertirPrioridad(const std::string& strPrioridad, int& prioridad) {
    if (!esNumeroEntero(strPrioridad)) return false;

    try {
        int val = std::stoi(strPrioridad);
        if (val < 1 || val > 10) return false; // Rango permitido: 1 (máx) a 10 (mín)
        prioridad = val;
        return true;
    } catch (...) {
        return false; // Captura desbordamiento de int (ej. demasiados dígitos)
    }
}

bool LectorCSV::convertirMontoACentavos(const std::string& strMonto, long long& montoCentavos) {
    if (strMonto.empty()) return false;

    size_t posPunto = strMonto.find('.');
    std::string enteroPart = "";
    std::string decimalPart = "";

    if (posPunto == std::string::npos) {
        enteroPart = strMonto;
        decimalPart = "00";
    } else {
        enteroPart = strMonto.substr(0, posPunto);
        decimalPart = strMonto.substr(posPunto + 1);

        // Rechazar más de 2 decimales (evita truncamiento silencioso)
        if (decimalPart.length() > 2) {
            return false;
        }
        if (decimalPart.length() == 1) {
            decimalPart += "0";
        }
    }

    if (!esNumeroEntero(enteroPart) || !esNumeroEntero(decimalPart)) {
        return false;
    }

    try {
        long long enteros  = std::stoll(enteroPart);
        long long centavos = std::stoll(decimalPart);

        // Prevenir desbordamiento aritmético antes de multiplicar por 100
        if (enteros > (LLONG_MAX - centavos) / 100) {
            return false;
        }

        long long total = (enteros * 100) + centavos;
        if (total <= 0) return false; // Todo pedido debe tener monto positivo

        montoCentavos = total;
        return true;
    } catch (...) {
        return false;
    }
}

// --------------------------------------------------
// LECTURA Y PROCESAMIENTO DEL CSV
// --------------------------------------------------

int LectorCSV::cargarArchivo(const std::string& rutaArchivo, GestorPedidos& gestor) {
    std::ifstream archivo(rutaArchivo);
    if (!archivo.is_open()) {
        std::cerr << "[ERROR LectorCSV] No se pudo abrir el archivo: " << rutaArchivo << "\n";
        return 0;
    }

    std::string linea;
    int lineaActual = 0;
    int exitosos = 0;
    int fallidos = 0;

    while (std::getline(archivo, linea)) {
        lineaActual++;
        linea = trim(linea);

        if (linea.empty() || linea == "idPedido,dniCliente,tipoEnvio,nivelPrioridad,montoDolares,fecha,hora,estado") {
            continue; }

        std::string col[8];
        int colIndex = 0;
        size_t inicio = 0;
        size_t fin = linea.find(',');

        while (fin != std::string::npos && colIndex < 7) {
            col[colIndex++] = trim(linea.substr(inicio, fin - inicio));
            inicio = fin + 1;
            fin = linea.find(',', inicio);
        }
        if (colIndex < 8) {
            col[colIndex++] = trim(linea.substr(inicio));
        }

        if (colIndex != 8) {
            std::cerr << "[Linea " << lineaActual << " OMITIDA] Columnas incorrectas (" << colIndex << "/8).\n";
            fallidos++;
            continue;
        }

        int prioridad = 0;
        long long montoCentavos = 0;

        if (col[0].empty() ||
            !esDniValido(col[1]) ||
            !esTipoEnvioValido(col[2]) ||
            !convertirPrioridad(col[3], prioridad) ||
            !convertirMontoACentavos(col[4], montoCentavos) ||
            !esFechaValida(col[5]) ||
            !esHoraValida(col[6]) ||
            !esEstadoValido(col[7])) {
            std::cerr << "[Linea " << lineaActual << " OMITIDA] Datos o formato invalidos.\n";
            fallidos++;
            continue;
        }

        if (gestor.registrarPedido(col[0], col[1], col[2], prioridad, montoCentavos, col[5], col[6], col[7])) {
            exitosos++;
        } else {
            fallidos++;
        }
    }

    archivo.close();

    std::cout << "\n=== RESUMEN DE CARGA MASIVA ===\n";
    std::cout << "Registros cargados: " << exitosos << "\n";
    std::cout << "Registros omitidos: " << fallidos << "\n";
    std::cout << "--------------------------------\n";

    return exitosos;
}
