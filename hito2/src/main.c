#include <stdio.h>
#include <stdlib.h>
#include "config.h"
#include "database.h"
#include "auth.h"
#include "menu.h"
#include "utils/logger.h"
#include "utils/memory.h"
#include "test_data.h"  // Incluir el nuevo archivo

int main() {
    // Inicializar sistema de memoria
    memory_init();
    
    // Cargar configuración
    Config config;
    if (!load_config("config/config.ini", &config)) {
        fprintf(stderr, "Error: No se pudo cargar la configuración.\n");
        return EXIT_FAILURE;
    }
    
    AdminConfig admin_config;
    if (!load_admin_config("config/admin.ini", &admin_config)) {
        fprintf(stderr, "Error: No se pudo cargar la configuración de administrador.\n");
        return EXIT_FAILURE;
    }
    
    // Validar configuración
    if (!validate_config(&config) || !validate_admin_config(&admin_config)) {
        fprintf(stderr, "Error: Configuración inválida.\n");
        return EXIT_FAILURE;
    }
    
    // Inicializar sistema de logging
    LogLevel log_level = log_level_from_string(config.log_level);
    if (!log_init(config.log_path, log_level)) {
        fprintf(stderr, "Error: No se pudo inicializar el sistema de logging.\n");
        return EXIT_FAILURE;
    }
    
    log_info("===== Iniciando CineGestion =====");
    
    // Inicializar base de datos
    if (!db_init(config.db_path)) {
        log_critical("No se pudo inicializar la base de datos.");
        log_close();
        return EXIT_FAILURE;
    }
    
    log_info("Base de datos inicializada correctamente.");
    
    // Inicializar datos de prueba
    printf("¿Desea inicializar datos de prueba? (S/N): ");
    char respuesta;
    scanf(" %c", &respuesta);
    while (getchar() != '\n'); // Limpiar buffer
    
    if (respuesta == 'S' || respuesta == 's') {
        if (!test_data_init()) {
            log_error("Error al inicializar datos de prueba.");
        } else {
            log_info("Datos de prueba inicializados correctamente.");
        }
    }
    
    // Inicializar sistema de autenticación
    auth_init();
    
    // Inicializar sistema de menús
    menu_init();
    
    // Ejecutar el menú principal
    menu_ejecutar();
    
    // Limpieza y finalización
    log_info("===== Finalizando CineGestion =====");
    db_close();
    log_close();
    memory_cleanup();
    
    return EXIT_SUCCESS;
}