#ifndef USUARIO_H
#define USUARIO_H

#include <stdbool.h>

// Tipos de usuario
typedef enum {
    USUARIO_CLIENTE,
    USUARIO_ADMINISTRADOR
} TipoUsuario;

// Estructura del usuario
typedef struct {
    int id;
    char nombre[100];
    char correo[100];
    char contrasena[100];
    char telefono[20];
    TipoUsuario tipo;
} Usuario;

// Funciones CRUD
bool usuario_crear(Usuario* usuario);
bool usuario_obtener_por_id(int id, Usuario* usuario);
bool usuario_obtener_por_correo(const char* correo, Usuario* usuario);
bool usuario_actualizar(Usuario* usuario);
bool usuario_eliminar(int id);
bool usuario_listar(Usuario** usuarios, int* num_usuarios);

// Funciones de autenticación
bool usuario_autenticar(const char* correo, const char* contrasena, Usuario* usuario);
bool usuario_cambiar_contrasena(int id, const char* nueva_contrasena);

// Funciones de conversión
const char* usuario_tipo_a_string(TipoUsuario tipo);
TipoUsuario usuario_string_a_tipo(const char* tipo_str);

// Funciones de validación
bool usuario_validar(Usuario* usuario);

// Funciones de memoria
void usuario_liberar_lista(Usuario* usuarios, int num_usuarios);

#endif // USUARIO_H