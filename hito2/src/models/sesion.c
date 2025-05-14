#include "sesion.h"
#include "pelicula.h"
#include "sala.h"
#include "../database.h"
#include "../utils/logger.h"
#include "../utils/memory.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

// Función auxiliar para callback de SQLite
static int sesion_callback(void* data, int argc, char** argv, char** column_names) {
    Sesion* sesion = (Sesion*)data;
    
    for (int i = 0; i < argc; i++) {
        if (strcmp(column_names[i], "ID") == 0) {
            sesion->id = atoi(argv[i]);
        } else if (strcmp(column_names[i], "Pelicula_ID") == 0) {
            sesion->pelicula_id = atoi(argv[i]);
        } else if (strcmp(column_names[i], "Sala_ID") == 0) {
            sesion->sala_id = atoi(argv[i]);
        } else if (strcmp(column_names[i], "HoraInicio") == 0) {
            strncpy(sesion->hora_inicio, argv[i] ? argv[i] : "", sizeof(sesion->hora_inicio) - 1);
        } else if (strcmp(column_names[i], "HoraFin") == 0) {
            strncpy(sesion->hora_fin, argv[i] ? argv[i] : "", sizeof(sesion->hora_fin) - 1);
        }
    }
    
    return 0;
}

// Función callback para listar sesiones
static int sesiones_listar_callback(void* data, int argc, char** argv, char** column_names) {
    struct {
        Sesion* sesiones;
        int max_sesiones;
        int* num_sesiones;
    }* callback_data = (void*)data;
    
    if (*callback_data->num_sesiones >= callback_data->max_sesiones) {
        // No hay más espacio para sesiones
        return 1;
    }
    
    Sesion* sesion = &callback_data->sesiones[*callback_data->num_sesiones];
    
    for (int i = 0; i < argc; i++) {
        if (strcmp(column_names[i], "ID") == 0) {
            sesion->id = atoi(argv[i]);
        } else if (strcmp(column_names[i], "Pelicula_ID") == 0) {
            sesion->pelicula_id = atoi(argv[i]);
        } else if (strcmp(column_names[i], "Sala_ID") == 0) {
            sesion->sala_id = atoi(argv[i]);
        } else if (strcmp(column_names[i], "HoraInicio") == 0) {
            strncpy(sesion->hora_inicio, argv[i] ? argv[i] : "", sizeof(sesion->hora_inicio) - 1);
        } else if (strcmp(column_names[i], "HoraFin") == 0) {
            strncpy(sesion->hora_fin, argv[i] ? argv[i] : "", sizeof(sesion->hora_fin) - 1);
        }
    }
    
    (*callback_data->num_sesiones)++;
    return 0;
}

// Crear una nueva sesión
bool sesion_crear(Sesion* sesion) {
    if (!sesion_validar(sesion)) {
        log_error("Datos de sesión inválidos");
        return false;
    }
    
    // Comprobar disponibilidad
    if (!sesion_comprobar_disponibilidad(sesion)) {
        log_error("La sala no está disponible en el horario especificado");
        return false;
    }
    
    char sql[512];
    snprintf(sql, sizeof(sql),
            "INSERT INTO Sesion (Pelicula_ID, Sala_ID, HoraInicio, HoraFin) "
            "VALUES (%d, %d, '%s', '%s');",
            sesion->pelicula_id, sesion->sala_id, 
            sesion->hora_inicio, sesion->hora_fin);
    
    if (db_execute(sql)) {
        sesion->id = db_last_insert_id();
        log_info("Sesión creada con ID: %d", sesion->id);
        return true;
    }
    
    log_error("Error al crear sesión");
    return false;
}

// Obtener sesión por ID
bool sesion_obtener_por_id(int id, Sesion* sesion) {
    char sql[256];
    snprintf(sql, sizeof(sql), "SELECT * FROM Sesion WHERE ID = %d;", id);
    
    memset(sesion, 0, sizeof(Sesion));
    return db_query(sql, sesion_callback, sesion);
}

// Actualizar una sesión existente
bool sesion_actualizar(Sesion* sesion) {
    if (!sesion_validar(sesion) || sesion->id <= 0) {
        log_error("Datos de sesión inválidos para actualización");
        return false;
    }
    
    // Comprobar disponibilidad (excluyendo esta sesión)
    if (!sesion_comprobar_disponibilidad(sesion)) {
        log_error("La sala no está disponible en el horario especificado");
        return false;
    }
    
    char sql[512];
    snprintf(sql, sizeof(sql),
            "UPDATE Sesion SET Pelicula_ID = %d, Sala_ID = %d, "
            "HoraInicio = '%s', HoraFin = '%s' "
            "WHERE ID = %d;",
            sesion->pelicula_id, sesion->sala_id,
            sesion->hora_inicio, sesion->hora_fin,
            sesion->id);
    
    if (db_execute(sql)) {
        log_info("Sesión actualizada con ID: %d", sesion->id);
        return true;
    }
    
    log_error("Error al actualizar sesión con ID: %d", sesion->id);
    return false;
}

// Eliminar una sesión
bool sesion_eliminar(int id) {
    char sql[256];
    snprintf(sql, sizeof(sql), "DELETE FROM Sesion WHERE ID = %d;", id);
    
    if (db_execute(sql)) {
        log_info("Sesión eliminada con ID: %d", id);
        return true;
    }
    
    log_error("Error al eliminar sesión con ID: %d", id);
    return false;
}

// Listar todas las sesiones
bool sesion_listar(Sesion** sesiones, int* num_sesiones) {
    // Primero, contar cuántas sesiones hay
    char sql_count[256] = "SELECT COUNT(*) FROM Sesion;";
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
        *sesiones = NULL;
        *num_sesiones = 0;
        return true;
    }
    
    // Asignar memoria para las sesiones
    *sesiones = (Sesion*)MEM_ALLOC(count * sizeof(Sesion));
    if (!*sesiones) {
        log_error("Error al asignar memoria para la lista de sesiones");
        return false;
    }
    
    // Consultar las sesiones
    char sql[256] = "SELECT * FROM Sesion ORDER BY HoraInicio;";
    
    struct {
        Sesion* sesiones;
        int max_sesiones;
        int* num_sesiones;
    } callback_data = {*sesiones, count, num_sesiones};
    
    *num_sesiones = 0;  // Inicializar el contador
    
    if (!db_query(sql, sesiones_listar_callback, &callback_data)) {
        MEM_FREE(*sesiones);
        *sesiones = NULL;
        *num_sesiones = 0;
        log_error("Error al consultar la lista de sesiones");
        return false;
    }
    
    log_info("Se listaron %d sesiones", *num_sesiones);
    return true;
}

// Buscar sesiones por película
bool sesion_buscar_por_pelicula(int pelicula_id, Sesion** sesiones, int* num_sesiones) {
    char sql_count[256];
    snprintf(sql_count, sizeof(sql_count), 
            "SELECT COUNT(*) FROM Sesion WHERE Pelicula_ID = %d;", pelicula_id);
    
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
        *sesiones = NULL;
        *num_sesiones = 0;
        return true;
    }
    
    // Asignar memoria para las sesiones
    *sesiones = (Sesion*)MEM_ALLOC(count * sizeof(Sesion));
    if (!*sesiones) {
        log_error("Error al asignar memoria para la búsqueda de sesiones");
        return false;
    }
    
    // Consultar las sesiones
    char sql[256];
    snprintf(sql, sizeof(sql), 
            "SELECT * FROM Sesion WHERE Pelicula_ID = %d ORDER BY HoraInicio;", pelicula_id);
    
    struct {
        Sesion* sesiones;
        int max_sesiones;
        int* num_sesiones;
    } callback_data = {*sesiones, count, num_sesiones};
    
    *num_sesiones = 0;  // Inicializar el contador
    
    if (!db_query(sql, sesiones_listar_callback, &callback_data)) {
        MEM_FREE(*sesiones);
        *sesiones = NULL;
        *num_sesiones = 0;
        log_error("Error al consultar la búsqueda de sesiones por película");
        return false;
    }
    
    log_info("Se encontraron %d sesiones para la película %d", *num_sesiones, pelicula_id);
    return true;
}

// Buscar sesiones por sala
bool sesion_buscar_por_sala(int sala_id, Sesion** sesiones, int* num_sesiones) {
    char sql_count[256];
    snprintf(sql_count, sizeof(sql_count), 
            "SELECT COUNT(*) FROM Sesion WHERE Sala_ID = %d;", sala_id);
    
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
        *sesiones = NULL;
        *num_sesiones = 0;
        return true;
    }
    
    // Asignar memoria para las sesiones
    *sesiones = (Sesion*)MEM_ALLOC(count * sizeof(Sesion));
    if (!*sesiones) {
        log_error("Error al asignar memoria para la búsqueda de sesiones");
        return false;
    }
    
    // Consultar las sesiones
    char sql[256];
    snprintf(sql, sizeof(sql), 
            "SELECT * FROM Sesion WHERE Sala_ID = %d ORDER BY HoraInicio;", sala_id);
    
    struct {
        Sesion* sesiones;
        int max_sesiones;
        int* num_sesiones;
    } callback_data = {*sesiones, count, num_sesiones};
    
    *num_sesiones = 0;  // Inicializar el contador
    
    if (!db_query(sql, sesiones_listar_callback, &callback_data)) {
        MEM_FREE(*sesiones);
        *sesiones = NULL;
        *num_sesiones = 0;
        log_error("Error al consultar la búsqueda de sesiones por sala");
        return false;
    }
    
    log_info("Se encontraron %d sesiones para la sala %d", *num_sesiones, sala_id);
    return true;
}

// Buscar sesiones por fecha
bool sesion_buscar_por_fecha(const char* fecha, Sesion** sesiones, int* num_sesiones) {
    char sql_count[512];
    snprintf(sql_count, sizeof(sql_count), 
            "SELECT COUNT(*) FROM Sesion WHERE HoraInicio LIKE '%s%%';", fecha);
    
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
        *sesiones = NULL;
        *num_sesiones = 0;
        return true;
    }
    
    // Asignar memoria para las sesiones
    *sesiones = (Sesion*)MEM_ALLOC(count * sizeof(Sesion));
    if (!*sesiones) {
        log_error("Error al asignar memoria para la búsqueda de sesiones");
        return false;
    }
    
    // Consultar las sesiones
    char sql[512];
    snprintf(sql, sizeof(sql), 
            "SELECT * FROM Sesion WHERE HoraInicio LIKE '%s%%' ORDER BY HoraInicio;", fecha);
    
    struct {
        Sesion* sesiones;
        int max_sesiones;
        int* num_sesiones;
    } callback_data = {*sesiones, count, num_sesiones};
    
    *num_sesiones = 0;  // Inicializar el contador
    
    if (!db_query(sql, sesiones_listar_callback, &callback_data)) {
        MEM_FREE(*sesiones);
        *sesiones = NULL;
        *num_sesiones = 0;
        log_error("Error al consultar la búsqueda de sesiones por fecha");
        return false;
    }
    
    log_info("Se encontraron %d sesiones para la fecha %s", *num_sesiones, fecha);
    return true;
}

// Validar datos de sesión
bool sesion_validar(Sesion* sesion) {
    if (!sesion) {
        return false;
    }
    
    // Validar que los campos obligatorios no estén vacíos
    if (sesion->pelicula_id <= 0 || 
        sesion->sala_id <= 0 || 
        strlen(sesion->hora_inicio) == 0 || 
        strlen(sesion->hora_fin) == 0) {
        return false;
    }
    
    // Verificar que la película existe
    Pelicula pelicula;
    if (!pelicula_obtener_por_id(sesion->pelicula_id, &pelicula)) {
        log_error("La película con ID %d no existe", sesion->pelicula_id);
        return false;
    }
    
    // Verificar que la sala existe
    Sala sala;
    if (!sala_obtener_por_id(sesion->sala_id, &sala)) {
        log_error("La sala con ID %d no existe", sesion->sala_id);
        return false;
    }
    
    // Verificar formato de fechas y que hora_fin sea posterior a hora_inicio
    struct tm tm_inicio, tm_fin;
    
    if (!sesion_convertir_str_a_time(sesion->hora_inicio, &tm_inicio) ||
        !sesion_convertir_str_a_time(sesion->hora_fin, &tm_fin)) {
        log_error("Formato de fecha inválido");
        return false;
    }
    
    time_t tiempo_inicio = mktime(&tm_inicio);
    time_t tiempo_fin = mktime(&tm_fin);
    
    if (tiempo_fin <= tiempo_inicio) {
        log_error("La hora de fin debe ser posterior a la hora de inicio");
        return false;
    }
    
    return true;
}

// Comprobar disponibilidad de la sala en el horario especificado
bool sesion_comprobar_disponibilidad(Sesion* sesion) {
    char sql[512];
    
    // Si estamos actualizando una sesión existente, excluirla de la comprobación
    if (sesion->id > 0) {
        snprintf(sql, sizeof(sql),
                "SELECT COUNT(*) FROM Sesion "
                "WHERE Sala_ID = %d AND ID != %d AND "
                "((HoraInicio <= '%s' AND HoraFin > '%s') OR "
                "(HoraInicio < '%s' AND HoraFin >= '%s') OR "
                "(HoraInicio >= '%s' AND HoraFin <= '%s'));",
                sesion->sala_id, sesion->id,
                sesion->hora_inicio, sesion->hora_inicio,
                sesion->hora_fin, sesion->hora_fin,
                sesion->hora_inicio, sesion->hora_fin);
    } else {
        snprintf(sql, sizeof(sql),
                "SELECT COUNT(*) FROM Sesion "
                "WHERE Sala_ID = %d AND "
                "((HoraInicio <= '%s' AND HoraFin > '%s') OR "
                "(HoraInicio < '%s' AND HoraFin >= '%s') OR "
                "(HoraInicio >= '%s' AND HoraFin <= '%s'));",
                sesion->sala_id,
                sesion->hora_inicio, sesion->hora_inicio,
                sesion->hora_fin, sesion->hora_fin,
                sesion->hora_inicio, sesion->hora_fin);
    }
    
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

// Calcular la duración de la sesión en minutos
int sesion_calcular_duracion_minutos(Sesion* sesion) {
    struct tm tm_inicio, tm_fin;
    
    if (!sesion_convertir_str_a_time(sesion->hora_inicio, &tm_inicio) ||
        !sesion_convertir_str_a_time(sesion->hora_fin, &tm_fin)) {
        return -1;
    }
    
    time_t tiempo_inicio = mktime(&tm_inicio);
    time_t tiempo_fin = mktime(&tm_fin);
    
    return (int)difftime(tiempo_fin, tiempo_inicio) / 60;
}

// Convertir cadena a estructura tm
bool sesion_convertir_str_a_time(const char* str_hora, struct tm* tm_hora) {
    if (!str_hora || !tm_hora) {
        return false;
    }
    
    memset(tm_hora, 0, sizeof(struct tm));
    
    // Formato esperado: "YYYY-MM-DD HH:MM:SS"
    if (sscanf(str_hora, "%d-%d-%d %d:%d:%d", 
              &tm_hora->tm_year, &tm_hora->tm_mon, &tm_hora->tm_mday,
              &tm_hora->tm_hour, &tm_hora->tm_min, &tm_hora->tm_sec) != 6) {
        return false;
    }
    
    tm_hora->tm_year -= 1900;  // Ajustar año
    tm_hora->tm_mon -= 1;      // Ajustar mes (0-11)
    
    return true;
}

// Convertir estructura tm a cadena
bool sesion_convertir_time_a_str(struct tm* tm_hora, char* str_hora, size_t tam) {
    if (!tm_hora || !str_hora) {
        return false;
    }
    
    snprintf(str_hora, tam, "%04d-%02d-%02d %02d:%02d:%02d",
            tm_hora->tm_year + 1900, tm_hora->tm_mon + 1, tm_hora->tm_mday,
            tm_hora->tm_hour, tm_hora->tm_min, tm_hora->tm_sec);
    
    return true;
}

// Liberar memoria de una lista de sesiones
void sesion_liberar_lista(Sesion* sesiones, int num_sesiones) {
    if (sesiones) {
        MEM_FREE(sesiones);
    }
}