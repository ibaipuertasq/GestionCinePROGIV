#include "usuario.h"
#include "../database.h"
#include "../utils/logger.h"
#include "../utils/memory.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// Función auxiliar para callback de SQLite
static int usuario_callback(void* data, int argc, char** argv, char** column_names) {
    Usuario* usuario = (Usuario*)data;
    
    for (int i = 0; i < argc; i++) {
        if (strcmp(column_names[i], "ID") == 0) {
            usuario->id = atoi(argv[i]);
        } else if (strcmp(column_names[i], "Nombre") == 0) {
            strncpy(usuario->nombre, argv[i] ? argv[i] : "", sizeof(usuario->nombre) - 1);
        } else if (strcmp(column_names[i], "CorreoElectronico") == 0) {
            strncpy(usuario->correo, argv[i] ? argv[i] : "", sizeof(usuario->correo) - 1);
        } else if (strcmp(column_names[i], "Contrasena") == 0) {
            strncpy(usuario->contrasena, argv[i] ? argv[i] : "", sizeof(usuario->contrasena) - 1);
        } else if (strcmp(column_names[i], "Telefono") == 0) {
            strncpy(usuario->telefono, argv[i] ? argv[i] : "", sizeof(usuario->telefono) - 1);
        } else if (strcmp(column_names[i], "TipoUsuario") == 0) {
            usuario->tipo = usuario_string_a_tipo(argv[i]);
        }
    }
    
    return 0;
}

// Función callback para listar usuarios
static int usuarios_listar_callback(void* data, int argc, char** argv, char** column_names) {
    struct {
        Usuario* usuarios;
        int max_usuarios;
        int* num_usuarios;
    }* callback_data = (void*)data;
    
    if (*callback_data->num_usuarios >= callback_data->max_usuarios) {
        // No hay más espacio para usuarios
        return 1;
    }
    
    Usuario* usuario = &callback_data->usuarios[*callback_data->num_usuarios];
    
    for (int i = 0; i < argc; i++) {
        if (strcmp(column_names[i], "ID") == 0) {
            usuario->id = atoi(argv[i]);
        } else if (strcmp(column_names[i], "Nombre") == 0) {
            strncpy(usuario->nombre, argv[i] ? argv[i] : "", sizeof(usuario->nombre) - 1);
        } else if (strcmp(column_names[i], "CorreoElectronico") == 0) {
            strncpy(usuario->correo, argv[i] ? argv[i] : "", sizeof(usuario->correo) - 1);
        } else if (strcmp(column_names[i], "Contrasena") == 0) {
            strncpy(usuario->contrasena, argv[i] ? argv[i] : "", sizeof(usuario->contrasena) - 1);
        } else if (strcmp(column_names[i], "Telefono") == 0) {
            strncpy(usuario->telefono, argv[i] ? argv[i] : "", sizeof(usuario->telefono) - 1);
        } else if (strcmp(column_names[i], "TipoUsuario") == 0) {
            usuario->tipo = usuario_string_a_tipo(argv[i]);
        }
    }
    
    (*callback_data->num_usuarios)++;
    return 0;
}

// Crear un nuevo usuario
bool usuario_crear(Usuario* usuario) {
    if (!usuario_validar(usuario)) {
        log_error("Datos de usuario inválidos");
        return false;
    }
    
    char sql[512];
    snprintf(sql, sizeof(sql),
            "INSERT INTO Usuarios (Nombre, CorreoElectronico, Contrasena, Telefono, TipoUsuario) "
            "VALUES ('%s', '%s', '%s', '%s', '%s');",
            usuario->nombre, usuario->correo, usuario->contrasena, 
            usuario->telefono, usuario_tipo_a_string(usuario->tipo));
    
    if (db_execute(sql)) {
        usuario->id = db_last_insert_id();
        log_info("Usuario creado con ID: %d", usuario->id);
        return true;
    }
    
    log_error("Error al crear usuario");
    return false;
}

// Obtener usuario por ID
bool usuario_obtener_por_id(int id, Usuario* usuario) {
    char sql[256];
    snprintf(sql, sizeof(sql), "SELECT * FROM Usuarios WHERE ID = %d;", id);
    
    memset(usuario, 0, sizeof(Usuario));
    return db_query(sql, usuario_callback, usuario);
}

// Obtener usuario por correo electrónico
bool usuario_obtener_por_correo(const char* correo, Usuario* usuario) {
    char sql[256];
    snprintf(sql, sizeof(sql), "SELECT * FROM Usuarios WHERE CorreoElectronico = '%s';", correo);
    
    memset(usuario, 0, sizeof(Usuario));
    return db_query(sql, usuario_callback, usuario);
}

// Actualizar un usuario existente
bool usuario_actualizar(Usuario* usuario) {
    if (!usuario_validar(usuario) || usuario->id <= 0) {
        log_error("Datos de usuario inválidos para actualización");
        return false;
    }
    
    char sql[512];
    snprintf(sql, sizeof(sql),
            "UPDATE Usuarios SET Nombre = '%s', CorreoElectronico = '%s', "
            "Contrasena = '%s', Telefono = '%s', TipoUsuario = '%s' "
            "WHERE ID = %d;",
            usuario->nombre, usuario->correo, usuario->contrasena, 
            usuario->telefono, usuario_tipo_a_string(usuario->tipo),
            usuario->id);
    
    if (db_execute(sql)) {
        log_info("Usuario actualizado con ID: %d", usuario->id);
        return true;
    }
    
    log_error("Error al actualizar usuario con ID: %d", usuario->id);
    return false;
}

// Eliminar un usuario
bool usuario_eliminar(int id) {
    char sql[256];
    snprintf(sql, sizeof(sql), "DELETE FROM Usuarios WHERE ID = %d;", id);
    
    if (db_execute(sql)) {
        log_info("Usuario eliminado con ID: %d", id);
        return true;
    }
    
    log_error("Error al eliminar usuario con ID: %d", id);
    return false;
}

// Listar todos los usuarios
bool usuario_listar(Usuario** usuarios, int* num_usuarios) {
    // Primero, contar cuántos usuarios hay
    char sql_count[256] = "SELECT COUNT(*) FROM Usuarios;";
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
        *usuarios = NULL;
        *num_usuarios = 0;
        return true;
    }
    
    // Asignar memoria para los usuarios
    *usuarios = (Usuario*)MEM_ALLOC(count * sizeof(Usuario));
    if (!*usuarios) {
        log_error("Error al asignar memoria para la lista de usuarios");
        return false;
    }
    
    // Consultar los usuarios
    char sql[256] = "SELECT * FROM Usuarios;";
    
    struct {
        Usuario* usuarios;
        int max_usuarios;
        int* num_usuarios;
    } callback_data = {*usuarios, count, num_usuarios};
    
    *num_usuarios = 0;  // Inicializar el contador
    
    if (!db_query(sql, usuarios_listar_callback, &callback_data)) {
        MEM_FREE(*usuarios);
        *usuarios = NULL;
        *num_usuarios = 0;
        log_error("Error al consultar la lista de usuarios");
        return false;
    }
    
    log_info("Se listaron %d usuarios", *num_usuarios);
    return true;
}

// Autenticar un usuario
bool usuario_autenticar(const char* correo, const char* contrasena, Usuario* usuario) {
    if (!usuario_obtener_por_correo(correo, usuario)) {
        log_warning("Intento de autenticación fallido: usuario no encontrado (%s)", correo);
        return false;
    }
    
    if (strcmp(usuario->contrasena, contrasena) != 0) {
        log_warning("Intento de autenticación fallido: contraseña incorrecta para el usuario %s", correo);
        return false;
    }
    
    log_info("Usuario autenticado: %s", correo);
    return true;
}

// Cambiar la contraseña de un usuario
bool usuario_cambiar_contrasena(int id, const char* nueva_contrasena) {
    if (id <= 0 || !nueva_contrasena || strlen(nueva_contrasena) == 0) {
        log_error("Datos inválidos para cambio de contraseña");
        return false;
    }
    
    char sql[256];
    snprintf(sql, sizeof(sql), 
            "UPDATE Usuarios SET Contrasena = '%s' WHERE ID = %d;",
            nueva_contrasena, id);
    
    if (db_execute(sql)) {
        log_info("Contraseña actualizada para el usuario con ID: %d", id);
        return true;
    }
    
    log_error("Error al actualizar contraseña para el usuario con ID: %d", id);
    return false;
}

// Convertir tipo de usuario a cadena
const char* usuario_tipo_a_string(TipoUsuario tipo) {
    switch (tipo) {
        case USUARIO_CLIENTE:
            return "Cliente";
        case USUARIO_ADMINISTRADOR:
            return "Administrador";
        default:
            return "Desconocido";
    }
}

// Convertir cadena a tipo de usuario
TipoUsuario usuario_string_a_tipo(const char* tipo_str) {
    if (strcmp(tipo_str, "Administrador") == 0) {
        return USUARIO_ADMINISTRADOR;
    } else {
        return USUARIO_CLIENTE;
    }
}

// Validar datos de usuario
bool usuario_validar(Usuario* usuario) {
    if (!usuario) {
        return false;
    }
    
    // Validar que campos obligatorios no estén vacíos
    if (strlen(usuario->nombre) == 0 || 
        strlen(usuario->correo) == 0 || 
        strlen(usuario->contrasena) == 0) {
        return false;
    }
    
    // Aquí se podrían añadir más validaciones (formato de correo, longitud mínima de contraseña, etc.)
    
    return true;
}

// Liberar memoria de una lista de usuarios
void usuario_liberar_lista(Usuario* usuarios, int num_usuarios) {
    if (usuarios) {
        MEM_FREE(usuarios);
    }
}