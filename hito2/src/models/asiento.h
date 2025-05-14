#ifndef ASIENTO_H
#define ASIENTO_H

#include <stdbool.h>

// Estados de asiento
typedef enum {
    ASIENTO_LIBRE,
    ASIENTO_OCUPADO
} EstadoAsiento;

// Estructura de asiento
typedef struct {
    int id;
    int sala_id;
    int numero;
    EstadoAsiento estado;
} Asiento;

// Funciones CRUD
bool asiento_obtener_por_id(int id, Asiento* asiento);
bool asiento_actualizar_estado(int id, EstadoAsiento estado);
bool asiento_listar_por_sala(int sala_id, Asiento** asientos, int* num_asientos);

// Funciones adicionales
bool asiento_reservar(int id);
bool asiento_liberar(int id);
bool asiento_esta_disponible(int id);

// Funciones de conversión
const char* asiento_estado_a_string(EstadoAsiento estado);
EstadoAsiento asiento_string_a_estado(const char* estado_str);

// Funciones de memoria
void asiento_liberar_lista(Asiento* asientos, int num_asientos);

#endif // ASIENTO_H