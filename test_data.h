#ifndef TEST_DATA_H
#define TEST_DATA_H

#include <stdbool.h>

// Función para inicializar datos de prueba
bool test_data_init();

// Funciones auxiliares para crear datos específicos
bool test_data_crear_usuarios();
bool test_data_crear_peliculas();
bool test_data_crear_salas();
bool test_data_crear_sesiones();
bool test_data_crear_ventas_ejemplo();

// Función para verificar si ya existen datos
bool test_data_verificar_datos_existentes();

// Función para limpiar todos los datos de prueba
bool test_data_limpiar();

#endif // TEST_DATA_H