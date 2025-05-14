#include "pelicula.h"
#include "../database.h"
#include "../utils/logger.h"
#include "../utils/memory.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// Función auxiliar para callback de SQLite
static int pelicula_callback(void* data, int argc, char** argv, char** column_names) {
    Pelicula* pelicula = (Pelicula*)data;
    
    for (int i = 0; i < argc; i++) {
        if (strcmp(column_names[i], "ID") == 0) {
            pelicula->id = atoi(argv[i]);
        } else if (strcmp(column_names[i], "Titulo") == 0) {
            strncpy(pelicula->titulo, argv[i] ? argv[i] : "", sizeof(pelicula->titulo) - 1);
        } else if (strcmp(column_names[i], "Duracion") == 0) {
            pelicula->duracion = atoi(argv[i]);
        } else if (strcmp(column_names[i], "Genero") == 0) {
            strncpy(pelicula->genero, argv[i] ? argv[i] : "", sizeof(pelicula->genero) - 1);
        }
    }
    
    return 0;
}

// Función callback para listar películas
static int peliculas_listar_callback(void* data, int argc, char** argv, char** column_names) {
    struct {
        Pelicula* peliculas;
        int max_peliculas;
        int* num_peliculas;
    }* callback_data = (void*)data;
    
    if (*callback_data->num_peliculas >= callback_data->max_peliculas) {
        // No hay más espacio para películas
        return 1;
    }
    
    Pelicula* pelicula = &callback_data->peliculas[*callback_data->num_peliculas];
    
    for (int i = 0; i < argc; i++) {
        if (strcmp(column_names[i], "ID") == 0) {
            pelicula->id = atoi(argv[i]);
        } else if (strcmp(column_names[i], "Titulo") == 0) {
            strncpy(pelicula->titulo, argv[i] ? argv[i] : "", sizeof(pelicula->titulo) - 1);
        } else if (strcmp(column_names[i], "Duracion") == 0) {
            pelicula->duracion = atoi(argv[i]);
        } else if (strcmp(column_names[i], "Genero") == 0) {
            strncpy(pelicula->genero, argv[i] ? argv[i] : "", sizeof(pelicula->genero) - 1);
        }
    }
    
    (*callback_data->num_peliculas)++;
    return 0;
}

// Crear una nueva película
bool pelicula_crear(Pelicula* pelicula) {
    if (!pelicula_validar(pelicula)) {
        log_error("Datos de película inválidos");
        return false;
    }
    
    char sql[512];
    snprintf(sql, sizeof(sql),
            "INSERT INTO Pelicula (Titulo, Duracion, Genero) "
            "VALUES ('%s', %d, '%s');",
            pelicula->titulo, pelicula->duracion, pelicula->genero);
    
    if (db_execute(sql)) {
        pelicula->id = db_last_insert_id();
        log_info("Película creada con ID: %d", pelicula->id);
        return true;
    }
    
    log_error("Error al crear película");
    return false;
}

// Obtener película por ID
bool pelicula_obtener_por_id(int id, Pelicula* pelicula) {
    char sql[256];
    snprintf(sql, sizeof(sql), "SELECT * FROM Pelicula WHERE ID = %d;", id);
    
    memset(pelicula, 0, sizeof(Pelicula));
    return db_query(sql, pelicula_callback, pelicula);
}

// Actualizar una película existente
bool pelicula_actualizar(Pelicula* pelicula) {
    if (!pelicula_validar(pelicula) || pelicula->id <= 0) {
        log_error("Datos de película inválidos para actualización");
        return false;
    }
    
    char sql[512];
    snprintf(sql, sizeof(sql),
            "UPDATE Pelicula SET Titulo = '%s', Duracion = %d, Genero = '%s' "
            "WHERE ID = %d;",
            pelicula->titulo, pelicula->duracion, pelicula->genero, pelicula->id);
    
    if (db_execute(sql)) {
        log_info("Película actualizada con ID: %d", pelicula->id);
        return true;
    }
    
    log_error("Error al actualizar película con ID: %d", pelicula->id);
    return false;
}

// Eliminar una película
bool pelicula_eliminar(int id) {
    char sql[256];
    snprintf(sql, sizeof(sql), "DELETE FROM Pelicula WHERE ID = %d;", id);
    
    if (db_execute(sql)) {
        log_info("Película eliminada con ID: %d", id);
        return true;
    }
    
    log_error("Error al eliminar película con ID: %d", id);
    return false;
}

// Listar todas las películas
bool pelicula_listar(Pelicula** peliculas, int* num_peliculas) {
    // Primero, contar cuántas películas hay
    char sql_count[256] = "SELECT COUNT(*) FROM Pelicula;";
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
        *peliculas = NULL;
        *num_peliculas = 0;
        return true;
    }
    
    // Asignar memoria para las películas
    *peliculas = (Pelicula*)MEM_ALLOC(count * sizeof(Pelicula));
    if (!*peliculas) {
        log_error("Error al asignar memoria para la lista de películas");
        return false;
    }
    
    // Consultar las películas
    char sql[256] = "SELECT * FROM Pelicula;";
    
    struct {
        Pelicula* peliculas;
        int max_peliculas;
        int* num_peliculas;
    } callback_data = {*peliculas, count, num_peliculas};
    
    *num_peliculas = 0;  // Inicializar el contador
    
    if (!db_query(sql, peliculas_listar_callback, &callback_data)) {
        MEM_FREE(*peliculas);
        *peliculas = NULL;
        *num_peliculas = 0;
        log_error("Error al consultar la lista de películas");
        return false;
    }
    
    log_info("Se listaron %d películas", *num_peliculas);
    return true;
}

// Buscar películas por título
bool pelicula_buscar_por_titulo(const char* titulo, Pelicula** peliculas, int* num_peliculas) {
    char sql_count[512];
    snprintf(sql_count, sizeof(sql_count), 
            "SELECT COUNT(*) FROM Pelicula WHERE Titulo LIKE '%%%s%%';", titulo);
    
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
        *peliculas = NULL;
        *num_peliculas = 0;
        return true;
    }
    
    // Asignar memoria para las películas
    *peliculas = (Pelicula*)MEM_ALLOC(count * sizeof(Pelicula));
    if (!*peliculas) {
        log_error("Error al asignar memoria para la búsqueda de películas");
        return false;
    }
    
    // Consultar las películas
    char sql[512];
    snprintf(sql, sizeof(sql), 
            "SELECT * FROM Pelicula WHERE Titulo LIKE '%%%s%%';", titulo);
    
    struct {
        Pelicula* peliculas;
        int max_peliculas;
        int* num_peliculas;
    } callback_data = {*peliculas, count, num_peliculas};
    
    *num_peliculas = 0;  // Inicializar el contador
    
    if (!db_query(sql, peliculas_listar_callback, &callback_data)) {
        MEM_FREE(*peliculas);
        *peliculas = NULL;
        *num_peliculas = 0;
        log_error("Error al consultar la búsqueda de películas por título");
        return false;
    }
    
    log_info("Se encontraron %d películas con título similar a '%s'", *num_peliculas, titulo);
    return true;
}

// Buscar películas por género
bool pelicula_buscar_por_genero(const char* genero, Pelicula** peliculas, int* num_peliculas) {
    char sql_count[512];
    snprintf(sql_count, sizeof(sql_count), 
            "SELECT COUNT(*) FROM Pelicula WHERE Genero LIKE '%%%s%%';", genero);
    
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
        *peliculas = NULL;
        *num_peliculas = 0;
        return true;
    }
    
    // Asignar memoria para las películas
    *peliculas = (Pelicula*)MEM_ALLOC(count * sizeof(Pelicula));
    if (!*peliculas) {
        log_error("Error al asignar memoria para la búsqueda de películas");
        return false;
    }
    
    // Consultar las películas
    char sql[512];
    snprintf(sql, sizeof(sql), 
            "SELECT * FROM Pelicula WHERE Genero LIKE '%%%s%%';", genero);
    
    struct {
        Pelicula* peliculas;
        int max_peliculas;
        int* num_peliculas;
    } callback_data = {*peliculas, count, num_peliculas};
    
    *num_peliculas = 0;  // Inicializar el contador
    
    if (!db_query(sql, peliculas_listar_callback, &callback_data)) {
        MEM_FREE(*peliculas);
        *peliculas = NULL;
        *num_peliculas = 0;
        log_error("Error al consultar la búsqueda de películas por género");
        return false;
    }
    
    log_info("Se encontraron %d películas con género similar a '%s'", *num_peliculas, genero);
    return true;
}

// Validar datos de película
bool pelicula_validar(Pelicula* pelicula) {
    if (!pelicula) {
        return false;
    }
    
    // Validar que campos obligatorios no estén vacíos
    if (strlen(pelicula->titulo) == 0 || 
        strlen(pelicula->genero) == 0 || 
        pelicula->duracion <= 0) {
        return false;
    }
    
    return true;
}

// Liberar memoria de una lista de películas
void pelicula_liberar_lista(Pelicula* peliculas, int num_peliculas) {
    if (peliculas) {
        MEM_FREE(peliculas);
    }
}