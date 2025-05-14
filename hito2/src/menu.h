#ifndef MENU_H
#define MENU_H

#include <stdbool.h>

// Tipos de menú
typedef enum {
    MENU_PRINCIPAL,
    MENU_AUTENTICACION,
    MENU_ADMIN,
    MENU_CLIENTE,
    MENU_PELICULAS,
    MENU_SALAS,
    MENU_SESIONES,
    MENU_VENTAS,
    MENU_REPORTES
} TipoMenu;

// Función para mostrar un menú específico
void menu_mostrar(TipoMenu tipo);

// Función para limpiar la pantalla
void menu_limpiar_pantalla();

// Función para mostrar un mensaje de error
void menu_mostrar_error(const char* mensaje);

// Función para mostrar un mensaje de éxito
void menu_mostrar_exito(const char* mensaje);

// Función para pausar y esperar a que el usuario presione una tecla
void menu_pausar();

// Función para leer una cadena de texto
void menu_leer_texto(char* buffer, int tamano, const char* prompt);

// Función para leer un número entero
int menu_leer_entero(const char* prompt, int min, int max);

// Función para leer un número decimal
double menu_leer_decimal(const char* prompt, double min, double max);

// Función para confirmar una acción (Sí/No)
bool menu_confirmar(const char* prompt);

// Inicializar el sistema de menús
void menu_init();

// Ejecutar el bucle principal del menú
void menu_ejecutar();

#endif // MENU_H