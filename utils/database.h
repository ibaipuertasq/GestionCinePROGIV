#ifndef DATABASE_H
#define DATABASE_H

#include "lib/sqlite3.h"
#include <stdbool.h>

// Estructura para manejar la conexión a la base de datos
typedef struct {
    sqlite3 *db;
    char *error_message;
    bool connected;
} Database;

// Inicializar la base de datos
bool db_init(const char* db_path);

// Abrir la conexión a la base de datos
bool db_open(const char* db_path);

// Cerrar la conexión a la base de datos
void db_close();

// Ejecutar una consulta SQL sin resultados (CREATE, INSERT, UPDATE, DELETE)
bool db_execute(const char* sql);

// Crear las tablas de la base de datos si no existen
bool db_create_tables();

// Backup de la base de datos
bool db_backup(const char* backup_path);

// Restaurar la base de datos desde un backup
bool db_restore(const char* backup_path);

// Obtener instancia de la base de datos
Database* get_database();

// Ejecutar una consulta SQL con callback para procesar resultados
bool db_query(const char* sql, int (*callback)(void*, int, char**, char**), void* data);

// Iniciar una transacción
bool db_begin_transaction();

// Confirmar una transacción
bool db_commit_transaction();

// Revertir una transacción
bool db_rollback_transaction();

// Último ID insertado
int db_last_insert_id();

// Obtener número de cambios en la última operación
int db_changes();

#endif // DATABASE_H