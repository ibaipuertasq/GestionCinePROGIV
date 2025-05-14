#ifndef CONFIG_H
#define CONFIG_H

#include <stdbool.h>

// Estructura para almacenar la configuración del sistema
typedef struct {
    // General
    char app_name[50];
    char version[10];
    
    // Database
    char db_path[100];
    char db_backup_path[100];
    
    // Logs
    char log_path[100];
    char log_level[10];
    
    // UI
    int max_menu_items;
    bool clear_screen;
} Config;

// Estructura para almacenar configuración del administrador
typedef struct {
    // Admin
    char username[50];
    char password[50];
    char email[100];
    
    // Security
    int session_timeout;
    int max_login_attempts;
} AdminConfig;

// Cargar la configuración desde los archivos
bool load_config(const char* config_path, Config* config);
bool load_admin_config(const char* admin_config_path, AdminConfig* admin_config);

// Obtener instancias de configuración
Config* get_config();
AdminConfig* get_admin_config();

// Validar la configuración
bool validate_config(Config* config);
bool validate_admin_config(AdminConfig* admin_config);

// Mostrar la configuración actual (utilidad para depuración)
void print_config(Config* config);
void print_admin_config(AdminConfig* admin_config);

// Guardar cambios en la configuración (opcional para Hito 2)
bool save_config(const char* config_path, Config* config);
bool save_admin_config(const char* admin_config_path, AdminConfig* admin_config);

#endif // CONFIG_H