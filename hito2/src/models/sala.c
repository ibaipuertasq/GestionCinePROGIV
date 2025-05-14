#include "sala.h"
#include "../database.h"
#include "../utils/logger.h"
#include "../utils/memory.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// Función auxiliar para callback de SQLite
static int sala_callback(void* data, int argc, char** argv, char** column_names) {
    Sala* sala = (Sala*)data;
    
    for (int i = 0; i < argc; i++) {
        if (strcmp(column_names[i], "ID") == 0) {
            sala->id = atoi(argv[i]);
        } else if (strcmp(column_names[i], "NumeroAsientos") == 0) {
            sala->numero_asientos = atoi(argv[i]);
        }
    }
    
    return 0;
}

// Función callback para listar salas
static int salas_listar_callback(void* data, int argc, char** argv, char** column_names) {
    struct {
        Sala* salas;
        int max_salas;
        int* num_salas;
    }* callback_data = (void*)data;
    
    if (*callback_data->num_salas >= callback_data->max_salas) {
        // No hay más espacio para salas
        return 1;
    }
    
    Sala* sala = &callback_data->salas[*callback_data->num_salas];
    
    for (int i = 0; i < argc; i++) {
        if (strcmp(column_names[i], "ID") == 0) {
            sala->id = atoi(argv[i]);
        } else if (strcmp(column_names[i], "NumeroAsientos") == 0) {
            sala->numero_asientos = atoi(argv[i]);
        }
    }
    
    (*callback_data->num_salas)++;
    return 0;
}

// Crear una nueva sala
bool sala_crear(Sala* sala) {
    if (!sala_validar(sala)) {
        log_error("Datos de sala inválidos");
        return false;
    }
    
    char sql[256];
    snprintf(sql, sizeof(sql),
            "INSERT INTO Sala (NumeroAsientos) VALUES (%d);",
            sala->numero_asientos);
    
    if (db_execute(sql)) {
        sala->id = db_last_insert_id();
        log_info("Sala creada con ID: %d", sala->id);
        
        // Crear asientos para la sala
        if (!sala_crear_asientos(sala->id)) {
            log_warning("No se pudieron crear todos los asientos para la sala ID: %d", sala->id);
        }
        
        return true;
    }
    
    log_error("Error al crear sala");
    return false;
}

// Obtener sala por ID
bool sala_obtener_por_id(int id, Sala* sala) {
    char sql[256];
    snprintf(sql, sizeof(sql), "SELECT * FROM Sala WHERE ID = %d;", id);
    
    memset(sala, 0, sizeof(Sala));
    return db_query(sql, sala_callback, sala);
}

// Actualizar una sala existente
bool sala_actualizar(Sala* sala) {
    if (!sala_validar(sala) || sala->id <= 0) {
        log_error("Datos de sala inválidos para actualización");
        return false;
    }
    
    // Obtener la sala actual para comparar
    Sala sala_actual;
    if (!sala_obtener_por_id(sala->id, &sala_actual)) {
        log_error("No se pudo obtener la sala actual para actualizar");
        return false;
    }
    
    // Si el número de asientos ha cambiado, es más complejo (hay que añadir o eliminar asientos)
    if (sala_actual.numero_asientos != sala->numero_asientos) {
        log_warning("Cambiar el número de asientos de una sala podría afectar a las sesiones y entradas existentes");
    }
    
    char sql[256];
    snprintf(sql, sizeof(sql),
            "UPDATE Sala SET NumeroAsientos = %d WHERE ID = %d;",
            sala->numero_asientos, sala->id);
    
    if (db_execute(sql)) {
        log_info("Sala actualizada con ID: %d", sala->id);
        return true;
    }
    
    log_error("Error al actualizar sala con ID: %d", sala->id);
    return false;
}

// Eliminar una sala
bool sala_eliminar(int id) {
    // Nota: Las restricciones de clave foránea en la base de datos se encargarán
    // de eliminar los asientos asociados (ON DELETE CASCADE)
    
    char sql[256];
    snprintf(sql, sizeof(sql), "DELETE FROM Sala WHERE ID = %d;", id);
    
    if (db_execute(sql)) {
        log_info("Sala eliminada con ID: %d", id);
        return true;
    }
    
    log_error("Error al eliminar sala con ID: %d", id);
    return false;
}

// Listar todas las salas
bool sala_listar(Sala** salas, int* num_salas) {
    // Primero, contar cuántas salas hay
    char sql_count[256] = "SELECT COUNT(*) FROM Sala;";
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
        *salas = NULL;
        *num_salas = 0;
        return true;
    }
    
    // Asignar memoria para las salas
    *salas = (Sala*)MEM_ALLOC(count * sizeof(Sala));
    if (!*salas) {
        log_error("Error al asignar memoria para la lista de salas");
        return false;
    }
    
    // Consultar las salas
    char sql[256] = "SELECT * FROM Sala;";
    
    struct {
        Sala* salas;
        int max_salas;
        int* num_salas;
    } callback_data = {*salas, count, num_salas};
    
    *num_salas = 0;  // Inicializar el contador
    
    if (!db_query(sql, salas_listar_callback, &callback_data)) {
        MEM_FREE(*salas);
        *salas = NULL;
        *num_salas = 0;
        log_error("Error al consultar la lista de salas");
        return false;
    }
    
    log_info("Se listaron %d salas", *num_salas);
    return true;
}

// Crear asientos para una sala
bool sala_crear_asientos(int sala_id) {
    // Obtener información de la sala
    Sala sala;
    if (!sala_obtener_por_id(sala_id, &sala)) {
        log_error("No se pudo obtener información de la sala para crear asientos");
        return false;
    }
    
    // Iniciar transacción para asegurar consistencia
    if (!db_begin_transaction()) {
        log_error("Error al iniciar transacción para crear asientos");
        return false;
    }
    
    bool exito = true;
    
    // Crear los asientos
    for (int i = 1; i <= sala.numero_asientos; i++) {
        char sql[256];
        snprintf(sql, sizeof(sql),
                "INSERT INTO Asiento (Sala_ID, Numero, Estado) VALUES (%d, %d, 'Libre');",
                sala_id, i);
        
        if (!db_execute(sql)) {
            log_error("Error al crear asiento %d para la sala %d", i, sala_id);
            exito = false;
            break;
        }
    }
    
    // Confirmar o revertir la transacción
    if (exito) {
        if (!db_commit_transaction()) {
            log_error("Error al confirmar transacción para crear asientos");
            db_rollback_transaction();
            return false;
        }
        log_info("Se crearon %d asientos para la sala %d", sala.numero_asientos, sala_id);
    } else {
        db_rollback_transaction();
        log_error("Se revirtió la transacción de creación de asientos");
        return false;
    }
    
    return exito;
}

// Contar asientos libres en una sala
int sala_contar_asientos_libres(int sala_id) {
    char sql[256];
    snprintf(sql, sizeof(sql),
            "SELECT COUNT(*) FROM Asiento WHERE Sala_ID = %d AND Estado = 'Libre';",
            sala_id);
    
    int count = 0;
    
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(get_database()->db, sql, -1, &stmt, NULL);
    
    if (rc != SQLITE_OK) {
        log_error("Error al preparar la consulta: %s", sqlite3_errmsg(get_database()->db));
        return -1;
    }
    
    rc = sqlite3_step(stmt);
    
    if (rc == SQLITE_ROW) {
        count = sqlite3_column_int(stmt, 0);
    }
    
    sqlite3_finalize(stmt);
    
    return count;
}

// Validar datos de sala
bool sala_validar(Sala* sala) {
    if (!sala) {
        return false;
    }
    
    // Validar que el número de asientos sea válido
    if (sala->numero_asientos <= 0 || sala->numero_asientos > 1000) {
        return false;
    }
    
    return true;
}

// Liberar memoria de una lista de salas
void sala_liberar_lista(Sala* salas, int num_salas) {
    if (salas) {
        MEM_FREE(salas);
    }
}