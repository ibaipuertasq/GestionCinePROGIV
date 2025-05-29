#ifndef SALA_H
#define SALA_H

#include <stdbool.h>

// Estructura de sala
typedef struct {
    int id;
    int numero_asientos;
} Sala;

// Funciones CRUD
bool sala_crear(Sala* sala);
bool sala_obtener_por_id(int id, Sala* sala);
bool sala_actualizar(Sala* sala);
bool sala_eliminar(int id);
bool sala_listar(Sala** salas, int* num_salas);

// Funciones adicionales
bool sala_crear_asientos(int sala_id);
int sala_contar_asientos_libres(int sala_id);
bool sala_validar(Sala* sala);

// Funciones de memoria
void sala_liberar_lista(Sala* salas, int num_salas);

#endif // SALA_H