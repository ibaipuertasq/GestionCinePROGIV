#include "asiento.h"
#include "../database.h"
#include "../utils/logger.h"
#include "../utils/memory.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// Función auxiliar para callback de SQLite
static int asiento_callback(void* data, int argc, char** argv, char** column_names) {
    Asiento* asiento = (Asiento*)data;
    
    for (int i = 0; i < argc; i++) {
        if (strcmp(column_names[i], "ID") == 0) {
            asiento->id = atoi(argv[i]);
        } else if (strcmp(column_names[i], "Sala_ID") == 0) {
            asiento->sala_id = atoi(argv[i]);
        } else if (strcmp(column_names[i], "Numero") == 0) {
            asiento->numero = atoi(argv[i]);
        } else if (strcmp(column_names[i], "Estado") == 0) {
            asiento->estado = asiento_string_a_estado(argv[i]);
        }
    }
    
    return 0;
}

// Función callback para listar asientos
static int asientos_listar_callback(void* data, int argc, char** argv, char** column_names) {
    struct {
        Asiento* asientos;
        int max_asientos;
        int* num_asientos;
    }* callback_data = (void*)data;
    
    if (*callback_data->num_asientos >= callback_data->max_asientos) {
        // No hay más espacio para asientos
        return 1;
    }
    
    Asiento* asiento = &callback_data->asientos[*callback_data->num_asientos];
    
    for (int i = 0; i < argc; i++) {
        if (strcmp(column_names[i], "ID") == 0) {
            asiento->id = atoi(argv[i]);
        } else if (strcmp(column_names[i], "Sala_ID") == 0) {
            asiento->sala_id = atoi(argv[i]);
        } else if (strcmp(column_names[i], "Numero") == 0) {
            asiento->numero = atoi(argv[i]);
        } else if (strcmp(column_names[i], "Estado") == 0) {
            asiento->estado = asiento_string_a_estado(argv[i]);
        }
    }
    
    (*callback_data->num_asientos)++;
    return 0;
}

// Obtener asiento por ID
bool asiento_obtener_por_id(int id, Asiento* asiento) {
    char sql[256];
    snprintf(sql, sizeof(sql), "SELECT * FROM Asiento WHERE ID = %d;", id);
    
    memset(asiento, 0, sizeof(Asiento));
    return db_query(sql, asiento_callback, asiento);
}

// Actualizar estado de un asiento
bool asiento_actualizar_estado(int id, EstadoAsiento estado) {
    char sql[256];
    snprintf(sql, sizeof(sql),
            "UPDATE Asiento SET Estado = '%s' WHERE ID = %d;",
            asiento_estado_a_string(estado), id);
    
    if (db_execute(sql)) {
        log_info("Estado del asiento %d actualizado a %s", id, asiento_estado_a_string(estado));
        return true;
    }
    
    log_error("Error al actualizar estado del asiento %d", id);
    return false;
}

// Listar asientos por sala
bool asiento_listar_por_sala(int sala_id, Asiento** asientos, int* num_asientos) {
    // Primero, contar cuántos asientos hay en la sala
    char sql_count[256];
    snprintf(sql_count, sizeof(sql_count), "SELECT COUNT(*) FROM Asiento WHERE Sala_ID = %d;", sala_id);
    
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
        *asientos = NULL;
        *num_asientos = 0;
        return true;
    }
    
    // Asignar memoria para los asientos
    *asientos = (Asiento*)MEM_ALLOC(count * sizeof(Asiento));
    if (!*asientos) {
        log_error("Error al asignar memoria para la lista de asientos");
        return false;
    }
    
    // Consultar los asientos
    char sql[256];
    snprintf(sql, sizeof(sql), "SELECT * FROM Asiento WHERE Sala_ID = %d ORDER BY Numero;", sala_id);
    
    struct {
        Asiento* asientos;
        int max_asientos;
        int* num_asientos;
    } callback_data = {*asientos, count, num_asientos};
    
    *num_asientos = 0;  // Inicializar el contador
    
    if (!db_query(sql, asientos_listar_callback, &callback_data)) {
        MEM_FREE(*asientos);
        *asientos = NULL;
        *num_asientos = 0;
        log_error("Error al consultar la lista de asientos");
        return false;
    }
    
    log_info("Se listaron %d asientos para la sala %d", *num_asientos, sala_id);
    return true;
}

// Reservar un asiento
bool asiento_reservar(int id) {
    return asiento_actualizar_estado(id, ASIENTO_OCUPADO);
}

// Liberar un asiento
bool asiento_liberar(int id) {
    return asiento_actualizar_estado(id, ASIENTO_LIBRE);
}

// Comprobar si un asiento está disponible
bool asiento_esta_disponible(int id) {
    Asiento asiento;
    
    if (!asiento_obtener_por_id(id, &asiento)) {
        log_error("No se pudo obtener el asiento %d", id);
        return false;
    }
    
    return asiento.estado == ASIENTO_LIBRE;
}

// Convertir estado de asiento a cadena
const char* asiento_estado_a_string(EstadoAsiento estado) {
    switch (estado) {
        case ASIENTO_LIBRE:
            return "Libre";
        case ASIENTO_OCUPADO:
            return "Ocupado";
        default:
            return "Desconocido";
    }
}

// Convertir cadena a estado de asiento
EstadoAsiento asiento_string_a_estado(const char* estado_str) {
    if (strcmp(estado_str, "Ocupado") == 0) {
        return ASIENTO_OCUPADO;
    } else {
        return ASIENTO_LIBRE;
    }
}

// Liberar memoria de una lista de asientos
void asiento_liberar_lista(Asiento* asientos, int num_asientos) {
    if (asientos) {
        MEM_FREE(asientos);
    }
}