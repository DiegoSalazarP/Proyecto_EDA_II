#ifndef LECTOR_CSV_H
#define LECTOR_CSV_H

#include "GestorPedidos.h"
#include <string>

class LectorCSV {
public:
    static int cargarPedidos(const std::string& rutaArchivo, GestorPedidos& gestor);
};

#endif // LECTOR_CSV_H