#include "LectorCSV.h"
#include <fstream>
#include <sstream>
#include <iostream>

int LectorCSV::cargarPedidos(const std::string& rutaArchivo, GestorPedidos& gestor) {
    std::ifstream archivo(rutaArchivo);
    if (!archivo.is_open()) {
        std::cerr << "Error: No se pudo abrir el archivo CSV en: " << rutaArchivo << std::endl;
        return 0;
    }

    std::string linea;
    int cargados = 0;
    bool esCabecera = true;

    while (std::getline(archivo, linea)) {
        if (linea.empty()) continue;
        if (!linea.empty() && linea.back() == '\r') {
            linea.pop_back();
        }

        if (esCabecera) {
            esCabecera = false;
            if (linea.find("id") == 0 || linea.find("ID") == 0) {
                continue;
            }
        }

        std::stringstream ss(linea);
        std::string id, dni, tipo, prioridadStr, montoStr, fecha, hora, estado;

        if (std::getline(ss, id, ',') &&
            std::getline(ss, dni, ',') &&
            std::getline(ss, tipo, ',') &&
            std::getline(ss, prioridadStr, ',') &&
            std::getline(ss, montoStr, ',') &&
            std::getline(ss, fecha, ',') &&
            std::getline(ss, hora, ',') &&
            std::getline(ss, estado, ',')) {

            try {
                int prioridad = std::stoi(prioridadStr);
                long long montoCentavos = static_cast<long long>(std::stod(montoStr) * 100.0 + 0.5);

                if (gestor.registrarPedido(id, dni, tipo, prioridad, montoCentavos, fecha, hora, estado)) {
                    cargados++;
                }
            } catch (...) {
                // Ignora filas malformadas
            }
        }
    }

    archivo.close();
    return cargados;
}