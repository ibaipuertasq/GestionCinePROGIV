#ifndef PELICULA_H
#define PELICULA_H

#include <stdbool.h>

// Estructura de película
typedef struct {
    int id;
    char titulo[200];
    int duracion;        // en minutos
    char genero[100];
} Pelicula;

// Funciones CRUD
bool pelicula_crear(Pelicula* pelicula);
bool pelicula_obtener_por_id(int id, Pelicula* pelicula);
bool pelicula_actualizar(Pelicula* pelicula);
bool pelicula_eliminar(int id);
bool pelicula_listar(Pelicula** peliculas, int* num_peliculas);

// Funciones de búsqueda
bool pelicula_buscar_por_titulo(const char* titulo, Pelicula** peliculas, int* num_peliculas);
bool pelicula_buscar_por_genero(const char* genero, Pelicula** peliculas, int* num_peliculas);

// Funciones de validación
bool pelicula_validar(Pelicula* pelicula);

// Funciones de memoria
void pelicula_liberar_lista(Pelicula* peliculas, int num_peliculas);

#endif // PELICULA_H