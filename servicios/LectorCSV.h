#ifndef LECTOR_CSV_H
#define LECTOR_CSV_H

#include <string>
#include "GestorPedidos.h"

class LectorCSV {
private:
    static std::string trim(const std::string& str);
    static bool esNumeroEntero(const std::string& str);

    // Validaciones de calendario
    static bool esBisiesto(int anio);
    static int diasEnMes(int mes, int anio);
    static bool esFechaValida(const std::string& fecha);
    static bool esHoraValida(const std::string& hora);

    // Validaciones de dominio y conversiones seguras
    static bool esDniValido(const std::string& dni);
    static bool esTipoEnvioValido(const std::string& tipo);
    static bool esEstadoValido(const std::string& estado);
    static bool convertirPrioridad(const std::string& strPrioridad, int& prioridad);
    static bool convertirMontoACentavos(const std::string& strMonto, long long& montoCentavos);

public:
    static int cargarArchivo(const std::string& rutaArchivo, GestorPedidos& gestor);
};

#endif // LECTOR_CSV_H
