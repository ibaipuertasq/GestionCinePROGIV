#include "venta.h"
#include "usuario.h"
#include "../database.h"
#include "../utils/logger.h"
#include "../utils/memory.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

// Función auxiliar para callback de SQLite
static int venta_callback(void* data, int argc, char** argv, char** column_names) {
    Venta* venta = (Venta*)data;
    
    for (int i = 0; i < argc; i++) {
        if (strcmp(column_names[i], "ID") == 0) {
            venta->id = atoi(argv[i]);
        } else if (strcmp(column_names[i], "Usuario_ID") == 0) {
            venta->usuario_id = atoi(argv[i]);
        } else if (strcmp(column_names[i], "Fecha") == 0) {
            strncpy(venta->fecha, argv[i] ? argv[i] : "", sizeof(venta->fecha) - 1);
        } else if (strcmp(column_names[i], "Descuento") == 0) {
            venta->descuento = atof(argv[i]);
        } else if (strcmp(column_names[i], "PrecioTotal") == 0) {
            venta->precio_total = atof(argv[i]);
        }
    }
    
    return 0;
}

// Función callback para listar ventas
static int ventas_listar_callback(void* data, int argc, char** argv, char** column_names) {
    struct {
        Venta* ventas;
        int max_ventas;
        int* num_ventas;
    }* callback_data = (void*)data;
    
    if (*callback_data->num_ventas >= callback_data->max_ventas) {
        // No hay más espacio para ventas
        return 1;
    }
    
    Venta* venta = &callback_data->ventas[*callback_data->num_ventas];
    
    for (int i = 0; i < argc; i++) {
        if (strcmp(column_names[i], "ID") == 0) {
            venta->id = atoi(argv[i]);
        } else if (strcmp(column_names[i], "Usuario_ID") == 0) {
            venta->usuario_id = atoi(argv[i]);
        } else if (strcmp(column_names[i], "Fecha") == 0) {
            strncpy(venta->fecha, argv[i] ? argv[i] : "", sizeof(venta->fecha) - 1);
        } else if (strcmp(column_names[i], "Descuento") == 0) {
            venta->descuento = atof(argv[i]);
        } else if (strcmp(column_names[i], "PrecioTotal") == 0) {
            venta->precio_total = atof(argv[i]);
        }
    }
    
    (*callback_data->num_ventas)++;
    return 0;
}

// Crear una nueva venta con sus billetes
bool venta_crear(Venta* venta, Billete* billetes, int num_billetes) {
    if (!venta_validar(venta) || !billetes || num_billetes <= 0) {
        log_error("Datos de venta inválidos");
        return false;
    }
    
    // Calcular el precio total
    venta->precio_total = venta_calcular_total(billetes, num_billetes, venta->descuento);
    
    // Establecer la fecha actual si no se proporcionó
    if (strlen(venta->fecha) == 0) {
        time_t now = time(NULL);
        struct tm* tm_info = localtime(&now);
        strftime(venta->fecha, sizeof(venta->fecha), "%Y-%m-%d %H:%M:%S", tm_info);
    }
    
    // Iniciar transacción
    if (!db_begin_transaction()) {
        log_error("Error al iniciar transacción para crear venta");
        return false;
    }
    
    // Insertar la venta
    char sql[512];
    snprintf(sql, sizeof(sql),
            "INSERT INTO Venta (Usuario_ID, Fecha, Descuento, PrecioTotal) "
            "VALUES (%d, '%s', %.2f, %.2f);",
            venta->usuario_id, venta->fecha, venta->descuento, venta->precio_total);
    
    if (!db_execute(sql)) {
        log_error("Error al crear la venta");
        db_rollback_transaction();
        return false;
    }
    
    venta->id = db_last_insert_id();
    
    // Insertar los billetes
    for (int i = 0; i < num_billetes; i++) {
        // Crear el billete si no existe
        if (billetes[i].id <= 0) {
            if (!billete_crear(&billetes[i])) {
                log_error("Error al crear el billete %d", i);
                db_rollback_transaction();
                return false;
            }
        }
        
        // Asociar el billete a la venta
        snprintf(sql, sizeof(sql),
                "INSERT INTO Venta_Billetes (Venta_ID, Billete_ID) "
                "VALUES (%d, %d);",
                venta->id, billetes[i].id);
        
        if (!db_execute(sql)) {
            log_error("Error al asociar el billete %d a la venta %d", billetes[i].id, venta->id);
            db_rollback_transaction();
            return false;
        }
    }
    
    // Confirmar transacción
    if (!db_commit_transaction()) {
        log_error("Error al confirmar transacción para crear venta");
        db_rollback_transaction();
        return false;
    }
    
    log_info("Venta creada con ID: %d, Total: %.2f", venta->id, venta->precio_total);
    return true;
}

// Obtener venta por ID
bool venta_obtener_por_id(int id, Venta* venta) {
    char sql[256];
    snprintf(sql, sizeof(sql), "SELECT * FROM Venta WHERE ID = %d;", id);
    
    memset(venta, 0, sizeof(Venta));
    return db_query(sql, venta_callback, venta);
}

// Eliminar una venta
bool venta_eliminar(int id) {
    // Obtener los billetes asociados para liberarlos
    Billete* billetes = NULL;
    int num_billetes = 0;
    
    if (!venta_obtener_billetes(id, &billetes, &num_billetes)) {
        log_error("Error al obtener los billetes de la venta %d", id);
        return false;
    }
    
    // Iniciar transacción
    if (!db_begin_transaction()) {
        log_error("Error al iniciar transacción para eliminar venta");
        billete_liberar_lista(billetes, num_billetes);
        return false;
    }
    
    // Eliminar los billetes
    for (int i = 0; i < num_billetes; i++) {
        if (!billete_eliminar(billetes[i].id)) {
            log_error("Error al eliminar el billete %d", billetes[i].id);
            db_rollback_transaction();
            billete_liberar_lista(billetes, num_billetes);
            return false;
        }
    }
    
    // Eliminar las relaciones en la tabla intermedia
    char sql[256];
    snprintf(sql, sizeof(sql), "DELETE FROM Venta_Billetes WHERE Venta_ID = %d;", id);
    
    if (!db_execute(sql)) {
        log_error("Error al eliminar las relaciones de venta-billetes");
        db_rollback_transaction();
        billete_liberar_lista(billetes, num_billetes);
        return false;
    }
    
    // Eliminar la venta
    snprintf(sql, sizeof(sql), "DELETE FROM Venta WHERE ID = %d;", id);
    
    if (!db_execute(sql)) {
        log_error("Error al eliminar venta con ID: %d", id);
        db_rollback_transaction();
        billete_liberar_lista(billetes, num_billetes);
        return false;
    }
    
    // Confirmar transacción
    if (!db_commit_transaction()) {
        log_error("Error al confirmar transacción para eliminar venta");
        db_rollback_transaction();
        billete_liberar_lista(billetes, num_billetes);
        return false;
    }
    
    billete_liberar_lista(billetes, num_billetes);
    log_info("Venta eliminada con ID: %d", id);
    return true;
}

// Listar ventas por usuario
bool venta_listar_por_usuario(int usuario_id, Venta** ventas, int* num_ventas) {
    char sql_count[256];
    snprintf(sql_count, sizeof(sql_count), 
            "SELECT COUNT(*) FROM Venta WHERE Usuario_ID = %d;", usuario_id);
    
    int count = 0;
    
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(get_database()->db, sql_count, -1, &stmt, NULL);
    
    if (rc != SQLITE_OK) {
        log_error("Error al preparar la consulta: %s", sqlite3_errmsg(get_database()->db));
        return false;
    }
    
    rc = sqlite3_step(stmt);
    
    if (rc == SQLITE_ROW) {
        count = sqlite3_column_int(stmt, 0);
    }
    
    sqlite3_finalize(stmt);
    
    if (count == 0) {
        *ventas = NULL;
        *num_ventas = 0;
        return true;
    }
    
    // Asignar memoria para las ventas
    *ventas = (Venta*)MEM_ALLOC(count * sizeof(Venta));
    if (!*ventas) {
        log_error("Error al asignar memoria para la lista de ventas");
        return false;
    }
    
    // Consultar las ventas
    char sql[256];
    snprintf(sql, sizeof(sql), 
            "SELECT * FROM Venta WHERE Usuario_ID = %d ORDER BY Fecha DESC;", usuario_id);
    
    struct {
        Venta* ventas;
        int max_ventas;
        int* num_ventas;
    } callback_data = {*ventas, count, num_ventas};
    
    *num_ventas = 0;  // Inicializar el contador
    
    if (!db_query(sql, ventas_listar_callback, &callback_data)) {
        MEM_FREE(*ventas);
        *ventas = NULL;
        *num_ventas = 0;
        log_error("Error al consultar la lista de ventas");
        return false;
    }
    
    log_info("Se listaron %d ventas para el usuario %d", *num_ventas, usuario_id);
    return true;
}

// Validar datos de venta
bool venta_validar(Venta* venta) {
    if (!venta) {
        return false;
    }
    
    // Validar que el usuario existe
    if (venta->usuario_id <= 0) {
        log_error("ID de usuario inválido");
        return false;
    }
    
    Usuario usuario;
    if (!usuario_obtener_por_id(venta->usuario_id, &usuario)) {
        log_error("El usuario con ID %d no existe", venta->usuario_id);
        return false;
    }
    
    // Validar descuento
    if (venta->descuento < 0 || venta->descuento > 100) {
        log_error("Descuento inválido: %.2f", venta->descuento);
        return false;
    }
    
    return true;
}

// Obtener los billetes de una venta
bool venta_obtener_billetes(int venta_id, Billete** billetes, int* num_billetes) {
    char sql_count[256];
    snprintf(sql_count, sizeof(sql_count), 
            "SELECT COUNT(*) FROM Venta_Billetes WHERE Venta_ID = %d;", venta_id);
    
    int count = 0;
    
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(get_database()->db, sql_count, -1, &stmt, NULL);
    
    if (rc != SQLITE_OK) {
        log_error("Error al preparar la consulta: %s", sqlite3_errmsg(get_database()->db));
        return false;
    }
    
    rc = sqlite3_step(stmt);
    
    if (rc == SQLITE_ROW) {
        count = sqlite3_column_int(stmt, 0);
    }
    
    sqlite3_finalize(stmt);
    
    if (count == 0) {
        *billetes = NULL;
        *num_billetes = 0;
        return true;
    }
    
    // Asignar memoria para los billetes
    *billetes = (Billete*)MEM_ALLOC(count * sizeof(Billete));
    if (!*billetes) {
        log_error("Error al asignar memoria para la lista de billetes");
        return false;
    }
    
    // Consultar los IDs de los billetes
    char sql[512];
    snprintf(sql, sizeof(sql), 
            "SELECT Billete_ID FROM Venta_Billetes WHERE Venta_ID = %d;", venta_id);
    
    sqlite3_stmt *stmt_ids;
    rc = sqlite3_prepare_v2(get_database()->db, sql, -1, &stmt_ids, NULL);
    
    if (rc != SQLITE_OK) {
        log_error("Error al preparar la consulta: %s", sqlite3_errmsg(get_database()->db));
        MEM_FREE(*billetes);
        *billetes = NULL;
        *num_billetes = 0;
        return false;
    }
    
    int i = 0;
    while (sqlite3_step(stmt_ids) == SQLITE_ROW && i < count) {
        int billete_id = sqlite3_column_int(stmt_ids, 0);
        
        // Obtener el billete
        if (!billete_obtener_por_id(billete_id, &(*billetes)[i])) {
            log_error("Error al obtener el billete %d", billete_id);
            sqlite3_finalize(stmt_ids);
            MEM_FREE(*billetes);
            *billetes = NULL;
            *num_billetes = 0;
            return false;
        }
        
        i++;
    }
    
    sqlite3_finalize(stmt_ids);
    *num_billetes = i;
    
    return true;
}

// Calcular el total de una venta
double venta_calcular_total(Billete* billetes, int num_billetes, double descuento) {
    double total = 0.0;
    
    for (int i = 0; i < num_billetes; i++) {
        total += billetes[i].precio;
    }
    
    // Aplicar descuento
    total = total * (1.0 - descuento / 100.0);
    
    return total;
}

// Liberar memoria de una lista de ventas
void venta_liberar_lista(Venta* ventas, int num_ventas) {
    if (ventas) {
        MEM_FREE(ventas);
    }
}