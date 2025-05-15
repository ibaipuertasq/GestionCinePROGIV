// bridge.h
#ifndef BRIDGE_H
#define BRIDGE_H

#include <vector>
#include <string>
#include "../common/models/pelicula.h"
#include "../common/models/sesion.h"

// Funciones de inicialización
bool bridge_init_db(const char* db_path);
void bridge_close_db();

// Funciones de autenticación
int bridge_login(const char* email, const char* password);
int bridge_user_get_type(int userId);
std::string bridge_user_get_name(int userId);
bool bridge_user_is_admin(int userId);

// Funciones de películas
bool bridge_pelicula_list(std::vector<Pelicula>* peliculas, int* num_peliculas);
bool bridge_pelicula_get_by_id(int id, Pelicula* pelicula);
bool bridge_pelicula_create(Pelicula* pelicula);
bool bridge_pelicula_update(Pelicula* pelicula);
bool bridge_pelicula_delete(int id);
bool bridge_pelicula_search_by_titulo(const char* titulo, std::vector<Pelicula>* peliculas, int* num_peliculas);
bool bridge_pelicula_search_by_genero(const char* genero, std::vector<Pelicula>* peliculas, int* num_peliculas);

// Funciones de sesiones
bool bridge_sesion_list(std::vector<Sesion>* sesiones, int* num_sesiones);
bool bridge_sesion_get_by_id(int id, Sesion* sesion);
bool bridge_sesion_create(Sesion* sesion);
bool bridge_sesion_update(Sesion* sesion);
bool bridge_sesion_delete(int id);
bool bridge_sesion_search_by_pelicula(int pelicula_id, std::vector<Sesion>* sesiones, int* num_sesiones);
bool bridge_sesion_search_by_sala(int sala_id, std::vector<Sesion>* sesiones, int* num_sesiones);
bool bridge_sesion_search_by_fecha(const char* fecha, std::vector<Sesion>* sesiones, int* num_sesiones);

// Funciones de salas
bool bridge_sala_list(std::vector<int>* salaIds, std::vector<int>* numAsientos, int* num_salas);
bool bridge_sala_get_by_id(int id, int* numAsientos);
bool bridge_asiento_list_by_sala(int sala_id, std::vector<int>* asientoIds, std::vector<int>* numeros, std::vector<bool>* disponibles, int* num_asientos);

// Funciones de billetes y ventas
bool bridge_billete_create(int sesion_id, int asiento_id, double precio);
bool bridge_billete_esta_disponible(int sesion_id, int asiento_id);
int bridge_venta_create(int usuario_id, int* sesion_ids, int* asiento_ids, int num_billetes, double descuento);
bool bridge_venta_list_by_user(int usuario_id, std::vector<int>* venta_ids, std::vector<std::string>* fechas, std::vector<double>* totales, int* num_ventas);
bool bridge_venta_get(int venta_id, int* usuario_id, std::string* fecha, double* descuento, double* total);
bool bridge_venta_get_billetes(int venta_id, std::vector<int>* sesion_ids, std::vector<int>* asiento_ids, std::vector<double>* precios, int* num_billetes);

#endif // BRIDGE_H