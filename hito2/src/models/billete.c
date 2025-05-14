#include "billete.h"
#include "asiento.h"
#include "sesion.h"
#include "../database.h"
#include "../utils/logger.h"
#include "../utils/memory.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// Función auxiliar para callback de SQLite
static int billete_callback(void* data, int argc, char** argv, char** column_names) {
    Billete* billete = (Billete*)data;
    
    for (int i = 0; i < argc; i++) {
        if (strcmp(column_names[i], "ID") == 0) {
            billete->id = atoi(argv[i]);
        } else if (strcmp(column_names[i], "Sesion_ID") == 0) {
            billete->sesion_id = atoi(argv[i]);
        } else if (strcmp(column_names[i], "Asiento_ID") == 0) {
            billete->asiento_id = atoi(argv[i]);
        } else if (strcmp(column_names[i], "Precio") == 0) {
            billete->precio = atof(argv[i]);
        }
    }
    
    return 0;
}

// Función callback para listar billetes
static int billetes_listar_callback(void* data, int argc, char** argv, char** column_names) {
    struct {
        Billete* billetes;
        int max_billetes;
        int* num_billetes;
    }* callback_data = (void*)data;
    
    if (*callback_data->num_billetes >= callback_data->max_billetes) {
        // No hay más espacio para billetes
        return 1;
    }
    
    Billete* billete = &callback_data->billetes[*callback_data->num_billetes];
    
    for (int i = 0; i < argc; i++) {
        if (strcmp(column_names[i], "ID") == 0) {
            billete->id = atoi(argv[i]);
        } else if (strcmp(column_names[i], "Sesion_ID") == 0) {
            billete->sesion_id = atoi(argv[i]);
        } else if (strcmp(column_names[i], "Asiento_ID") == 0) {
            billete->asiento_id = atoi(argv[i]);
        } else if (strcmp(column_names[i], "Precio") == 0) {
            billete->precio = atof(argv[i]);
        }
    }
    
    (*callback_data->num_billetes)++;
    return 0;
}

// Crear un nuevo billete
bool billete_crear(Billete* billete) {
    if (!billete_validar(billete)) {
        log_error("Datos de billete inválidos");
        return false;
    }
    
    // Comprobar si el asiento está disponible para esta sesión
    if (!billete_esta_disponible(billete->sesion_id, billete->asiento_id)) {
        log_error("El asiento no está disponible para esta sesión");
        return false;
    }
    
    // Iniciar transacción
    if (!db_begin_transaction()) {
        log_error("Error al iniciar transacción para crear billete");
        return false;
    }
    
    char sql[512];
    snprintf(sql, sizeof(sql),
            "INSERT INTO Billete (Sesion_ID, Asiento_ID, Precio) "
            "VALUES (%d, %d, %.2f);",
            billete->sesion_id, billete->asiento_id, billete->precio);
    
    if (!db_execute(sql)) {
        log_error("Error al crear billete");
        db_rollback_transaction();
        return false;
    }
    
    billete->id = db_last_insert_id();
    
    // Marcar el asiento como ocupado
    if (!asiento_reservar(billete->asiento_id)) {
        log_error("Error al reservar el asiento");
        db_rollback_transaction();
        return false;
    }
    
    // Confirmar transacción
    if (!db_commit_transaction()) {
        log_error("Error al confirmar transacción para crear billete");
        db_rollback_transaction();
        return false;
    }
    
    log_info("Billete creado con ID: %d", billete->id);
    return true;
}

// Obtener billete por ID
bool billete_obtener_por_id(int id, Billete* billete) {
    char sql[256];
    snprintf(sql, sizeof(sql), "SELECT * FROM Billete WHERE ID = %d;", id);
    
    memset(billete, 0, sizeof(Billete));
    return db_query(sql, billete_callback, billete);
}

// Actualizar un billete existente
bool billete_actualizar(Billete* billete) {
    if (!billete_validar(billete) || billete->id <= 0) {
        log_error("Datos de billete inválidos para actualización");
        return false;
    }
    
    // Obtener el billete actual para comparar
    Billete billete_actual;
    if (!billete_obtener_por_id(billete->id, &billete_actual)) {
        log_error("No se pudo obtener el billete actual para actualizar");
        return false;
    }
    
    // Si se cambia el asiento, comprobar disponibilidad
    if (billete_actual.asiento_id != billete->asiento_id) {
        if (!billete_esta_disponible(billete->sesion_id, billete->asiento_id)) {
            log_error("El nuevo asiento no está disponible para esta sesión");
            return false;
        }
    }
    
    // Iniciar transacción
    if (!db_begin_transaction()) {
        log_error("Error al iniciar transacción para actualizar billete");
        return false;
    }
    
    char sql[512];
    snprintf(sql, sizeof(sql),
            "UPDATE Billete SET Sesion_ID = %d, Asiento_ID = %d, Precio = %.2f "
            "WHERE ID = %d;",
            billete->sesion_id, billete->asiento_id, billete->precio, billete->id);
    
    if (!db_execute(sql)) {
        log_error("Error al actualizar billete con ID: %d", billete->id);
        db_rollback_transaction();
        return false;
    }
    
    // Si se cambia el asiento, liberar el anterior y reservar el nuevo
    if (billete_actual.asiento_id != billete->asiento_id) {
        if (!asiento_liberar(billete_actual.asiento_id)) {
            log_error("Error al liberar el asiento anterior");
            db_rollback_transaction();
            return false;
        }
        
        if (!asiento_reservar(billete->asiento_id)) {
            log_error("Error al reservar el nuevo asiento");
            db_rollback_transaction();
            return false;
        }
    }
    
    // Confirmar transacción
    if (!db_commit_transaction()) {
        log_error("Error al confirmar transacción para actualizar billete");
        db_rollback_transaction();
        return false;
    }
    
    log_info("Billete actualizado con ID: %d", billete->id);
    return true;
}

// Eliminar un billete
bool billete_eliminar(int id) {
    // Obtener el billete para liberar su asiento
    Billete billete;
    if (!billete_obtener_por_id(id, &billete)) {
        log_error("No se pudo obtener el billete para eliminar");
        return false;
    }
    
    // Iniciar transacción
    if (!db_begin_transaction()) {
        log_error("Error al iniciar transacción para eliminar billete");
        return false;
    }
    
    char sql[256];
    snprintf(sql, sizeof(sql), "DELETE FROM Billete WHERE ID = %d;", id);
    
    if (!db_execute(sql)) {
        log_error("Error al eliminar billete con ID: %d", id);
        db_rollback_transaction();
        return false;
    }
    
    // Liberar el asiento
    if (!asiento_liberar(billete.asiento_id)) {
        log_error("Error al liberar el asiento");
        db_rollback_transaction();
        return false;
    }
    
    // Confirmar transacción
    if (!db_commit_transaction()) {
        log_error("Error al confirmar transacción para eliminar billete");
        db_rollback_transaction();
        return false;
    }
    
    log_info("Billete eliminado con ID: %d", id);
    return true;
}

// Listar billetes por sesión
bool billete_listar_por_sesion(int sesion_id, Billete** billetes, int* num_billetes) {
    char sql_count[256];
    snprintf(sql_count, sizeof(sql_count), 
            "SELECT COUNT(*) FROM Billete WHERE Sesion_ID = %d;", sesion_id);
    
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
    
    // Consultar los billetes
    char sql[256];
    snprintf(sql, sizeof(sql), 
            "SELECT * FROM Billete WHERE Sesion_ID = %d;", sesion_id);
    
    struct {
        Billete* billetes;
        int max_billetes;
        int* num_billetes;
    } callback_data = {*billetes, count, num_billetes};
    
    *num_billetes = 0;  // Inicializar el contador
    
    if (!db_query(sql, billetes_listar_callback, &callback_data)) {
        MEM_FREE(*billetes);
        *billetes = NULL;
        *num_billetes = 0;
        log_error("Error al consultar la lista de billetes");
        return false;
    }
    
    log_info("Se listaron %d billetes para la sesión %d", *num_billetes, sesion_id);
    return true;
}

// Validar datos de billete
bool billete_validar(Billete* billete) {
    if (!billete) {
        return false;
    }
    
    // Validar que los campos obligatorios sean válidos
    if (billete->sesion_id <= 0 || 
        billete->asiento_id <= 0 || 
        billete->precio < 0) {
        return false;
    }
    
    // Verificar que la sesión existe
    Sesion sesion;
    if (!sesion_obtener_por_id(billete->sesion_id, &sesion)) {
        log_error("La sesión con ID %d no existe", billete->sesion_id);
        return false;
    }
    
    // Verificar que el asiento existe
    Asiento asiento;
    if (!asiento_obtener_por_id(billete->asiento_id, &asiento)) {
        log_error("El asiento con ID %d no existe", billete->asiento_id);
        return false;
    }
    
    // Verificar que el asiento pertenece a la sala de la sesión
    if (asiento.sala_id != sesion.sala_id) {
        log_error("El asiento %d no pertenece a la sala %d de la sesión %d",
                 asiento.id, sesion.sala_id, sesion.id);
        return false;
    }
    
    return true;
}

// Comprobar si un asiento está disponible para una sesión
bool billete_esta_disponible(int sesion_id, int asiento_id) {
    // Comprobar que el asiento está libre
    if (!asiento_esta_disponible(asiento_id)) {
        return false;
    }
    
    // Comprobar que no hay un billete para ese asiento en esa sesión
    char sql[256];
    snprintf(sql, sizeof(sql), 
            "SELECT COUNT(*) FROM Billete WHERE Sesion_ID = %d AND Asiento_ID = %d;", 
            sesion_id, asiento_id);
    
    int count = 0;
    
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(get_database()->db, sql, -1, &stmt, NULL);
    
    if (rc != SQLITE_OK) {
        log_error("Error al preparar la consulta: %s", sqlite3_errmsg(get_database()->db));
        return false;
    }
    
    rc = sqlite3_step(stmt);
    
    if (rc == SQLITE_ROW) {
        count = sqlite3_column_int(stmt, 0);
    }
    
    sqlite3_finalize(stmt);
    
    return count == 0;
}

// Calcular el precio base de un billete para una sesión
double billete_calcular_precio_base(int sesion_id) {
    // Para simplificar, se establece un precio base fijo
    // En una implementación real, podría depender de varios factores
    // como el día de la semana, la hora, el tipo de película, etc.
    return 8.50;
}

// Liberar memoria de una lista de billetes
void billete_liberar_lista(Billete* billetes, int num_billetes) {
    if (billetes) {
        MEM_FREE(billetes);
    }
}