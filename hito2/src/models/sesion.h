#ifndef SESION_H
#define SESION_H

#include <stdbool.h>
#include <time.h>

// Estructura de sesión
typedef struct {
    int id;
    int pelicula_id;
    int sala_id;
    char hora_inicio[20]; // Formato: "YYYY-MM-DD HH:MM:SS"
    char hora_fin[20];    // Formato: "YYYY-MM-DD HH:MM:SS"
} Sesion;

// Funciones CRUD
bool sesion_crear(Sesion* sesion);
bool sesion_obtener_por_id(int id, Sesion* sesion);
bool sesion_actualizar(Sesion* sesion);
bool sesion_eliminar(int id);
bool sesion_listar(Sesion** sesiones, int* num_sesiones);

// Funciones de búsqueda
bool sesion_buscar_por_pelicula(int pelicula_id, Sesion** sesiones, int* num_sesiones);
bool sesion_buscar_por_sala(int sala_id, Sesion** sesiones, int* num_sesiones);
bool sesion_buscar_por_fecha(const char* fecha, Sesion** sesiones, int* num_sesiones);

// Funciones adicionales
bool sesion_validar(Sesion* sesion);
bool sesion_comprobar_disponibilidad(Sesion* sesion); // Comprueba si la sala está disponible en ese horario
int sesion_calcular_duracion_minutos(Sesion* sesion); // Calcula la duración en minutos

// Funciones de ayuda
bool sesion_convertir_str_a_time(const char* str_hora, struct tm* tm_hora);
bool sesion_convertir_time_a_str(struct tm* tm_hora, char* str_hora, size_t tam);

// Funciones de memoria
void sesion_liberar_lista(Sesion* sesiones, int num_sesiones);

#endif // SESION_H