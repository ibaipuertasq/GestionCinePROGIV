#ifndef AUTH_H
#define AUTH_H

#include "models/usuario.h"
#include "models/sesion.h"

#include <stdbool.h>

// Estructura para mantener la sesión actual
typedef struct {
    bool autenticado;
    Usuario usuario;
    time_t tiempo_inicio;
    time_t tiempo_ultimo_acceso;
} SesionAuth;

// Inicializar el sistema de autenticación
void auth_init();

// Intentar iniciar sesión
bool auth_login(const char* correo, const char* contrasena);

// Cerrar sesión
void auth_logout();

// Comprobar si hay una sesión activa
bool auth_sesion_activa();

// Obtener usuario de la sesión actual
Usuario* auth_obtener_usuario_actual();

// Comprobar si la sesión ha expirado
bool auth_sesion_expirada();

// Refrescar el tiempo de la sesión
void auth_refrescar_sesion();

// Comprobar si el usuario actual es administrador
bool auth_es_administrador();

#endif // AUTH_H