#include "database.h"
#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Instancia global de la base de datos
static Database g_database = {NULL, NULL, false};

// Inicializar la base de datos
bool db_init(const char* db_path) {
    // Abrir conexión a la base de datos
    if (!db_open(db_path)) {
        fprintf(stderr, "Error: No se pudo abrir la base de datos.\n");
        return false;
    }
    
    // Crear las tablas si no existen
    if (!db_create_tables()) {
        fprintf(stderr, "Error: No se pudieron crear las tablas de la base de datos.\n");
        db_close();
        return false;
    }
    
    return true;
}

// Abrir la conexión a la base de datos
bool db_open(const char* db_path) {
    if (g_database.connected) {
        return true; // Ya está conectado
    }
    
    int rc = sqlite3_open(db_path, &g_database.db);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Error al abrir la base de datos: %s\n", sqlite3_errmsg(g_database.db));
        sqlite3_close(g_database.db);
        g_database.db = NULL;
        return false;
    }
    
    g_database.connected = true;
    return true;
}

// Cerrar la conexión a la base de datos
void db_close() {
    if (g_database.db) {
        sqlite3_close(g_database.db);
        g_database.db = NULL;
    }
    
    if (g_database.error_message) {
        sqlite3_free(g_database.error_message);
        g_database.error_message = NULL;
    }
    
    g_database.connected = false;
}

// Ejecutar una consulta SQL sin resultados
bool db_execute(const char* sql) {
    if (!g_database.connected || !g_database.db) {
        fprintf(stderr, "Error: No hay conexión a la base de datos.\n");
        return false;
    }
    
    char* error_message = NULL;
    int rc = sqlite3_exec(g_database.db, sql, NULL, NULL, &error_message);
    
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Error SQL: %s\n", error_message);
        sqlite3_free(error_message);
        return false;
    }
    
    return true;
}

// Crear las tablas de la base de datos si no existen
bool db_create_tables() {
    // Tabla Usuarios
    const char* sql_usuarios = 
        "CREATE TABLE IF NOT EXISTS Usuarios ("
        "ID INTEGER PRIMARY KEY AUTOINCREMENT,"
        "Nombre TEXT NOT NULL,"
        "CorreoElectronico TEXT UNIQUE NOT NULL,"
        "Contrasena TEXT NOT NULL,"
        "Telefono TEXT,"
        "TipoUsuario TEXT CHECK(TipoUsuario IN ('Administrador', 'Cliente')) NOT NULL"
        ");";
    
    // Tabla Película
    const char* sql_pelicula = 
        "CREATE TABLE IF NOT EXISTS Pelicula ("
        "ID INTEGER PRIMARY KEY AUTOINCREMENT,"
        "Titulo TEXT NOT NULL,"
        "Duracion INTEGER NOT NULL,"
        "Genero TEXT NOT NULL"
        ");";
    
    // Tabla Sala
    const char* sql_sala = 
        "CREATE TABLE IF NOT EXISTS Sala ("
        "ID INTEGER PRIMARY KEY AUTOINCREMENT,"
        "NumeroAsientos INTEGER NOT NULL"
        ");";
    
    // Tabla Asiento
    const char* sql_asiento = 
        "CREATE TABLE IF NOT EXISTS Asiento ("
        "ID INTEGER PRIMARY KEY AUTOINCREMENT,"
        "Sala_ID INTEGER NOT NULL,"
        "Numero INTEGER NOT NULL,"
        "Estado TEXT CHECK(Estado IN ('Libre', 'Ocupado')) NOT NULL DEFAULT 'Libre',"
        "FOREIGN KEY (Sala_ID) REFERENCES Sala(ID) ON DELETE CASCADE,"
        "UNIQUE(Sala_ID, Numero)"
        ");";
    
    // Tabla Sesión
    const char* sql_sesion = 
        "CREATE TABLE IF NOT EXISTS Sesion ("
        "ID INTEGER PRIMARY KEY AUTOINCREMENT,"
        "Pelicula_ID INTEGER NOT NULL,"
        "Sala_ID INTEGER NOT NULL,"
        "HoraInicio TEXT NOT NULL,"
        "HoraFin TEXT NOT NULL,"
        "FOREIGN KEY (Pelicula_ID) REFERENCES Pelicula(ID) ON DELETE CASCADE,"
        "FOREIGN KEY (Sala_ID) REFERENCES Sala(ID) ON DELETE CASCADE"
        ");";
    
    // Tabla Billete
    const char* sql_billete = 
        "CREATE TABLE IF NOT EXISTS Billete ("
        "ID INTEGER PRIMARY KEY AUTOINCREMENT,"
        "Sesion_ID INTEGER NOT NULL,"
        "Asiento_ID INTEGER NOT NULL,"
        "Precio REAL NOT NULL,"
        "FOREIGN KEY (Sesion_ID) REFERENCES Sesion(ID) ON DELETE CASCADE,"
        "FOREIGN KEY (Asiento_ID) REFERENCES Asiento(ID) ON DELETE CASCADE,"
        "UNIQUE(Sesion_ID, Asiento_ID)"
        ");";
    
    // Tabla Venta
    const char* sql_venta = 
        "CREATE TABLE IF NOT EXISTS Venta ("
        "ID INTEGER PRIMARY KEY AUTOINCREMENT,"
        "Usuario_ID INTEGER NOT NULL,"
        "Fecha TEXT NOT NULL,"
        "Descuento REAL DEFAULT 0,"
        "PrecioTotal REAL NOT NULL,"
        "FOREIGN KEY (Usuario_ID) REFERENCES Usuarios(ID) ON DELETE CASCADE"
        ");";
    
    // Tabla intermedia Venta_Billetes
    const char* sql_venta_billetes = 
        "CREATE TABLE IF NOT EXISTS Venta_Billetes ("
        "Venta_ID INTEGER NOT NULL,"
        "Billete_ID INTEGER NOT NULL,"
        "PRIMARY KEY (Venta_ID, Billete_ID),"
        "FOREIGN KEY (Venta_ID) REFERENCES Venta(ID) ON DELETE CASCADE,"
        "FOREIGN KEY (Billete_ID) REFERENCES Billete(ID) ON DELETE CASCADE"
        ");";
    
    // Crear todas las tablas
    if (!db_execute(sql_usuarios) ||
        !db_execute(sql_pelicula) ||
        !db_execute(sql_sala) ||
        !db_execute(sql_asiento) ||
        !db_execute(sql_sesion) ||
        !db_execute(sql_billete) ||
        !db_execute(sql_venta) ||
        !db_execute(sql_venta_billetes)) {
        return false;
    }
    
    // Verificar si existe el usuario administrador por defecto
    const char* sql_check_admin = 
        "SELECT COUNT(*) FROM Usuarios WHERE TipoUsuario = 'Administrador';";
    
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(g_database.db, sql_check_admin, -1, &stmt, NULL);
    
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Error al preparar la consulta: %s\n", sqlite3_errmsg(g_database.db));
        return false;
    }
    
    rc = sqlite3_step(stmt);
    
    if (rc == SQLITE_ROW) {
        int count = sqlite3_column_int(stmt, 0);
        sqlite3_finalize(stmt);
        
        // Si no hay administradores, crear el administrador por defecto
        if (count == 0) {
            AdminConfig* admin_config = get_admin_config();
            
            char sql_insert_admin[512];
            snprintf(sql_insert_admin, sizeof(sql_insert_admin),
                    "INSERT INTO Usuarios (Nombre, CorreoElectronico, Contrasena, TipoUsuario) "
                    "VALUES ('%s', '%s', '%s', 'Administrador');",
                    admin_config->username, admin_config->email, admin_config->password);
            
            if (!db_execute(sql_insert_admin)) {
                return false;
            }
        }
    } else {
        sqlite3_finalize(stmt);
        fprintf(stderr, "Error al verificar el usuario administrador.\n");
        return false;
    }
    
    return true;
}

// Realizar backup de la base de datos
bool db_backup(const char* backup_path) {
    if (!g_database.connected || !g_database.db) {
        fprintf(stderr, "Error: No hay conexión a la base de datos.\n");
        return false;
    }
    
    sqlite3 *backup_db;
    int rc = sqlite3_open(backup_path, &backup_db);
    
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Error al abrir la base de datos de backup: %s\n", sqlite3_errmsg(backup_db));
        sqlite3_close(backup_db);
        return false;
    }
    
    sqlite3_backup *backup = sqlite3_backup_init(backup_db, "main", g_database.db, "main");
    
    if (backup) {
        sqlite3_backup_step(backup, -1);
        sqlite3_backup_finish(backup);
    }
    
    rc = sqlite3_errcode(backup_db);
    sqlite3_close(backup_db);
    
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Error en el backup: %s\n", sqlite3_errmsg(backup_db));
        return false;
    }
    
    return true;
}

// Restaurar la base de datos desde un backup
bool db_restore(const char* backup_path) {
    if (!g_database.connected || !g_database.db) {
        fprintf(stderr, "Error: No hay conexión a la base de datos.\n");
        return false;
    }
    
    sqlite3 *backup_db;
    int rc = sqlite3_open(backup_path, &backup_db);
    
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Error al abrir la base de datos de backup: %s\n", sqlite3_errmsg(backup_db));
        sqlite3_close(backup_db);
        return false;
    }
    
    sqlite3_backup *backup = sqlite3_backup_init(g_database.db, "main", backup_db, "main");
    
    if (backup) {
        sqlite3_backup_step(backup, -1);
        sqlite3_backup_finish(backup);
    }
    
    rc = sqlite3_errcode(g_database.db);
    sqlite3_close(backup_db);
    
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Error en la restauración: %s\n", sqlite3_errmsg(g_database.db));
        return false;
    }
    
    return true;
}

// Obtener instancia de la base de datos
Database* get_database() {
    return &g_database;
}

// Ejecutar una consulta SQL con callback para procesar resultados
bool db_query(const char* sql, int (*callback)(void*, int, char**, char**), void* data) {
    if (!g_database.connected || !g_database.db) {
        fprintf(stderr, "Error: No hay conexión a la base de datos.\n");
        return false;
    }
    
    char* error_message = NULL;
    int rc = sqlite3_exec(g_database.db, sql, callback, data, &error_message);
    
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Error SQL: %s\n", error_message);
        sqlite3_free(error_message);
        return false;
    }
    
    return true;
}

// Iniciar una transacción
bool db_begin_transaction() {
    return db_execute("BEGIN TRANSACTION;");
}

// Confirmar una transacción
bool db_commit_transaction() {
    return db_execute("COMMIT;");
}

// Revertir una transacción
bool db_rollback_transaction() {
    return db_execute("ROLLBACK;");
}

// Último ID insertado
int db_last_insert_id() {
    if (!g_database.connected || !g_database.db) {
        fprintf(stderr, "Error: No hay conexión a la base de datos.\n");
        return -1;
    }
    
    return sqlite3_last_insert_rowid(g_database.db);
}

// Obtener número de cambios en la última operación
int db_changes() {
    if (!g_database.connected || !g_database.db) {
        fprintf(stderr, "Error: No hay conexión a la base de datos.\n");
        return -1;
    }
    
    return sqlite3_changes(g_database.db);
}