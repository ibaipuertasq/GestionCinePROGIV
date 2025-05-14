#ifndef VENTA_H
#define VENTA_H

#include <stdbool.h>
#include "billete.h"

// Estructura de venta
typedef struct {
    int id;
    int usuario_id;
    char fecha[20];        // Formato: "YYYY-MM-DD HH:MM:SS"
    double descuento;      // Porcentaje de descuento (0-100)
    double precio_total;
} Venta;

// Funciones CRUD
bool venta_crear(Venta* venta, Billete* billetes, int num_billetes);
bool venta_obtener_por_id(int id, Venta* venta);
bool venta_eliminar(int id);
bool venta_listar_por_usuario(int usuario_id, Venta** ventas, int* num_ventas);

// Funciones adicionales
bool venta_validar(Venta* venta);
bool venta_obtener_billetes(int venta_id, Billete** billetes, int* num_billetes);
double venta_calcular_total(Billete* billetes, int num_billetes, double descuento);

// Funciones de memoria
void venta_liberar_lista(Venta* ventas, int num_ventas);

#endif // VENTA_H