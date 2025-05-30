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

// Funcion para mostrar un menu especifico
void menu_mostrar(TipoMenu tipo);

// Funcion para limpiar la pantalla
void menu_limpiar_pantalla();

// Funcion para mostrar un mensaje de error
void menu_mostrar_error(const char* mensaje);

// Funcion para mostrar un mensaje de exito
void menu_mostrar_exito(const char* mensaje);

// Funcion para pausar y esperar a que el usuario presione una tecla
void menu_pausar();

// Funcion para leer una cadena de texto
void menu_leer_texto(char* buffer, int tamano, const char* prompt);

// Funcion para leer un numero entero
int menu_leer_entero(const char* prompt, int min, int max);

// Funcion para leer un numero decimal
double menu_leer_decimal(const char* prompt, double min, double max);

// Funcion para confirmar una accion (Si/No)
bool menu_confirmar(const char* prompt);

// Inicializar el sistema de menus
void menu_init();

// Ejecutar el bucle principal del meno
void menu_ejecutar();

#endif // MENU_H
