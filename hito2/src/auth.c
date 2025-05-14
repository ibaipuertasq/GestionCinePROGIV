#include "auth.h"
#include "config.h"
#include "utils/logger.h"
#include <string.h>
#include <time.h>

// Sesión global
static SesionAuth g_sesion = {false, {0}, 0, 0};

// Inicializar el sistema de autenticación
void auth_init() {
    memset(&g_sesion, 0, sizeof(SesionAuth));
    g_sesion.autenticado = false;
    log_debug("Sistema de autenticación inicializado");
}

// Intentar iniciar sesión
bool auth_login(const char* correo, const char* contrasena) {
    // Si ya hay una sesión activa, cerrarla primero
    if (g_sesion.autenticado) {
        auth_logout();
    }
    
    if (usuario_autenticar(correo, contrasena, &g_sesion.usuario)) {
        g_sesion.autenticado = true;
        g_sesion.tiempo_inicio = time(NULL);
        g_sesion.tiempo_ultimo_acceso = g_sesion.tiempo_inicio;
        
        log_info("Inicio de sesión exitoso para el usuario: %s", correo);
        return true;
    }
    
    log_warning("Intento de inicio de sesión fallido para el usuario: %s", correo);
    return false;
}

// Cerrar sesión
void auth_logout() {
    if (g_sesion.autenticado) {
        log_info("Cierre de sesión para el usuario: %s", g_sesion.usuario.correo);
        
        g_sesion.autenticado = false;
        memset(&g_sesion.usuario, 0, sizeof(Usuario));
        g_sesion.tiempo_inicio = 0;
        g_sesion.tiempo_ultimo_acceso = 0;
    }
}

// Comprobar si hay una sesión activa
bool auth_sesion_activa() {
    if (!g_sesion.autenticado) {
        return false;
    }
    
    // Comprobar si la sesión ha expirado
    if (auth_sesion_expirada()) {
        log_info("La sesión ha expirado automáticamente");
        auth_logout();
        return false;
    }
    
    return true;
}

// Obtener usuario de la sesión actual
Usuario* auth_obtener_usuario_actual() {
    if (auth_sesion_activa()) {
        return &g_sesion.usuario;
    }
    
    return NULL;
}

// Comprobar si la sesión ha expirado
bool auth_sesion_expirada() {
    if (!g_sesion.autenticado) {
        return true;
    }
    
    AdminConfig* admin_config = get_admin_config();
    time_t tiempo_actual = time(NULL);
    time_t tiempo_maximo = g_sesion.tiempo_ultimo_acceso + (admin_config->session_timeout * 60);
    
    return tiempo_actual > tiempo_maximo;
}

// Refrescar el tiempo de la sesión
void auth_refrescar_sesion() {
    if (g_sesion.autenticado) {
        g_sesion.tiempo_ultimo_acceso = time(NULL);
        log_debug("Sesión refrescada para el usuario: %s", g_sesion.usuario.correo);
    }
}

// Comprobar si el usuario actual es admi    nistrador
bool auth_es_administrador() {
    if (!auth_sesion_activa()) {
        return false;
    }
    
    return g_sesion.usuario.tipo == USUARIO_ADMINISTRADOR;
}