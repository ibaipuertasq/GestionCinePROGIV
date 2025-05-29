#ifndef BILLETE_H
#define BILLETE_H

#include <stdbool.h>

// Estructura de billete
typedef struct {
    int id;
    int sesion_id;
    int asiento_id;
    double precio;
} Billete;

// Funciones CRUD
bool billete_crear(Billete* billete);
bool billete_obtener_por_id(int id, Billete* billete);
bool billete_actualizar(Billete* billete);
bool billete_eliminar(int id);
bool billete_listar_por_sesion(int sesion_id, Billete** billetes, int* num_billetes);

// Funciones adicionales
bool billete_validar(Billete* billete);
bool billete_esta_disponible(int sesion_id, int asiento_id);
double billete_calcular_precio_base(int sesion_id);

// Funciones de memoria
void billete_liberar_lista(Billete* billetes, int num_billetes);

#endif // BILLETE_H