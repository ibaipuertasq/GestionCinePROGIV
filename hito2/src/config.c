#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// Variables estáticas globales para almacenar las configuraciones
static Config g_config;
static AdminConfig g_admin_config;
static bool g_config_loaded = false;
static bool g_admin_config_loaded = false;

// Función auxiliar para recortar espacios en blanco
static void trim(char* str) {
    if (!str) return;
    
    // Eliminar espacios al final
    int len = strlen(str);
    while (len > 0 && isspace(str[len - 1])) {
        str[--len] = '\0';
    }
    
    // Eliminar espacios al principio
    char* start = str;
    while (*start && isspace(*start)) {
        start++;
    }
    
    if (start != str) {
        memmove(str, start, len - (start - str) + 1);
    }
}

// Función auxiliar para leer un valor de la configuración
static bool get_value(const char* line, const char* key, char* value, size_t value_size) {
    size_t key_len = strlen(key);
    
    // Verificar si la línea comienza con la clave seguida de '='
    if (strncmp(line, key, key_len) == 0 && line[key_len] == '=') {
        // Copiar el valor después del '='
        strncpy(value, line + key_len + 1, value_size - 1);
        value[value_size - 1] = '\0';
        trim(value);
        return true;
    }
    
    return false;
}

// Cargar la configuración del sistema
bool load_config(const char* config_path, Config* config) {
    // Construir la ruta con el prefijo "hito2/"
    char full_path[256] = "hito2/";
    strcat(full_path, config_path);
    
    FILE* file = fopen(full_path, "r");
    if (!file) {
        fprintf(stderr, "Error: No se pudo abrir el archivo de configuración: %s\n", full_path);
        return false;
    }
    
    char line[256];
    char section[50] = "";
    
    // Establecer valores predeterminados
    strcpy(config->app_name, "CineGestion");
    strcpy(config->version, "1.0");
    strcpy(config->db_path, "data/cine.db");
    strcpy(config->db_backup_path, "data/backup/cine_backup.db");
    strcpy(config->log_path, "logs/system.log");
    strcpy(config->log_level, "INFO");
    config->max_menu_items = 10;
    config->clear_screen = true;
    
    while (fgets(line, sizeof(line), file)) {
        // Eliminar espacios y saltos de línea
        trim(line);
        
        // Ignorar líneas en blanco y comentarios
        if (line[0] == '\0' || line[0] == '#' || line[0] == ';') {
            continue;
        }
        
        // Comprobar si es una sección
        if (line[0] == '[' && line[strlen(line) - 1] == ']') {
            // Extraer el nombre de la sección
            strncpy(section, line + 1, sizeof(section) - 1);
            section[strlen(section) - 1] = '\0';
            continue;
        }
        
        // Procesar claves y valores según la sección
        if (strcmp(section, "general") == 0) {
            char value[100];
            if (get_value(line, "app_name", value, sizeof(value))) {
                strncpy(config->app_name, value, sizeof(config->app_name) - 1);
            } else if (get_value(line, "version", value, sizeof(value))) {
                strncpy(config->version, value, sizeof(config->version) - 1);
            }
        } else if (strcmp(section, "database") == 0) {
            char value[100];
            if (get_value(line, "db_path", value, sizeof(value))) {
                strncpy(config->db_path, value, sizeof(config->db_path) - 1);
            } else if (get_value(line, "db_backup_path", value, sizeof(value))) {
                strncpy(config->db_backup_path, value, sizeof(config->db_backup_path) - 1);
            }
        } else if (strcmp(section, "logs") == 0) {
            char value[100];
            if (get_value(line, "log_path", value, sizeof(value))) {
                strncpy(config->log_path, value, sizeof(config->log_path) - 1);
            } else if (get_value(line, "log_level", value, sizeof(value))) {
                strncpy(config->log_level, value, sizeof(config->log_level) - 1);
            }
        } else if (strcmp(section, "ui") == 0) {
            char value[100];
            if (get_value(line, "max_menu_items", value, sizeof(value))) {
                config->max_menu_items = atoi(value);
            } else if (get_value(line, "clear_screen", value, sizeof(value))) {
                if (strcmp(value, "true") == 0 || strcmp(value, "1") == 0) {
                    config->clear_screen = true;
                } else {
                    config->clear_screen = false;
                }
            }
        }
    }
    
    fclose(file);
    g_config_loaded = true;
    memcpy(&g_config, config, sizeof(Config));
    
    return true;
}

// Cargar la configuración de administrador
bool load_admin_config(const char* admin_config_path, AdminConfig* admin_config) {
    // Construir la ruta con el prefijo "hito2/"
    char full_path[256] = "hito2/";
    strcat(full_path, admin_config_path);
    
    FILE* file = fopen(full_path, "r");
    if (!file) {
        fprintf(stderr, "Error: No se pudo abrir el archivo de configuración de administrador: %s\n", full_path);
        return false;
    }
    
    char line[256];
    char section[50] = "";
    
    // Establecer valores predeterminados
    strcpy(admin_config->username, "admin");
    strcpy(admin_config->password, "admin123");
    strcpy(admin_config->email, "admin@cinegestion.com");
    admin_config->session_timeout = 30;
    admin_config->max_login_attempts = 3;
    
    while (fgets(line, sizeof(line), file)) {
        // Eliminar espacios y saltos de línea
        trim(line);
        
        // Ignorar líneas en blanco y comentarios
        if (line[0] == '\0' || line[0] == '#' || line[0] == ';') {
            continue;
        }
        
        // Comprobar si es una sección
        if (line[0] == '[' && line[strlen(line) - 1] == ']') {
            // Extraer el nombre de la sección
            strncpy(section, line + 1, sizeof(section) - 1);
            section[strlen(section) - 1] = '\0';
            continue;
        }
        
        // Procesar claves y valores según la sección
        if (strcmp(section, "admin") == 0) {
            char value[100];
            if (get_value(line, "username", value, sizeof(value))) {
                strncpy(admin_config->username, value, sizeof(admin_config->username) - 1);
            } else if (get_value(line, "password", value, sizeof(value))) {
                strncpy(admin_config->password, value, sizeof(admin_config->password) - 1);
            } else if (get_value(line, "email", value, sizeof(value))) {
                strncpy(admin_config->email, value, sizeof(admin_config->email) - 1);
            }
        } else if (strcmp(section, "security") == 0) {
            char value[100];
            if (get_value(line, "session_timeout", value, sizeof(value))) {
                admin_config->session_timeout = atoi(value);
            } else if (get_value(line, "max_login_attempts", value, sizeof(value))) {
                admin_config->max_login_attempts = atoi(value);
            }
        }
    }
    
    fclose(file);
    g_admin_config_loaded = true;
    memcpy(&g_admin_config, admin_config, sizeof(AdminConfig));
    
    return true;
}

// Obtener instancia única de configuración
Config* get_config() {
    if (!g_config_loaded) {
        Config config;
        // No necesitas añadir "hito2/" aquí, ya que load_config ya lo hace
        if (!load_config("config/config.ini", &config)) {
            fprintf(stderr, "Error: No se pudo cargar la configuración. Usando valores por defecto.\n");
            
            // Inicializa una configuración por defecto
            strcpy(config.app_name, "CineGestion");
            strcpy(config.version, "1.0");
            strcpy(config.db_path, "data/cine.db");
            strcpy(config.db_backup_path, "data/backup/cine_backup.db");
            strcpy(config.log_path, "logs/system.log");
            strcpy(config.log_level, "INFO");
            config.max_menu_items = 10;
            config.clear_screen = true;
            
            // Guardar valores por defecto en la estructura global
            g_config_loaded = true;
            memcpy(&g_config, &config, sizeof(Config));
        }
    }
    return &g_config;
}

// Obtener instancia única de configuración de administrador
AdminConfig* get_admin_config() {
    if (!g_admin_config_loaded) {
        AdminConfig admin_config;
        // No necesitas añadir "hito2/" aquí, ya que load_admin_config ya lo hace
        if (!load_admin_config("config/admin.ini", &admin_config)) {
            fprintf(stderr, "Error: No se pudo cargar la configuración de administrador. Usando valores por defecto.\n");
            
            // Inicializa una configuración por defecto
            strcpy(admin_config.username, "admin");
            strcpy(admin_config.password, "admin123");
            strcpy(admin_config.email, "admin@cinegestion.com");
            admin_config.session_timeout = 30;
            admin_config.max_login_attempts = 3;
            
            // Guardar valores por defecto en la estructura global
            g_admin_config_loaded = true;
            memcpy(&g_admin_config, &admin_config, sizeof(AdminConfig));
        }
    }
    return &g_admin_config;
}

// Validar la configuración del sistema
bool validate_config(Config* config) {
    // Validar que los campos obligatorios no estén vacíos
    if (strlen(config->app_name) == 0 || 
        strlen(config->version) == 0 || 
        strlen(config->db_path) == 0 || 
        strlen(config->log_path) == 0 || 
        strlen(config->log_level) == 0) {
        return false;
    }
    
    // Validar el nivel de log
    if (strcmp(config->log_level, "DEBUG") != 0 && 
        strcmp(config->log_level, "INFO") != 0 && 
        strcmp(config->log_level, "WARNING") != 0 && 
        strcmp(config->log_level, "ERROR") != 0 && 
        strcmp(config->log_level, "CRITICAL") != 0) {
        return false;
    }
    
    // Validar valores numéricos
    if (config->max_menu_items <= 0) {
        return false;
    }
    
    return true;
}

// Validar la configuración de administrador
bool validate_admin_config(AdminConfig* admin_config) {
    // Validar que los campos obligatorios no estén vacíos
    if (strlen(admin_config->username) == 0 || 
        strlen(admin_config->password) == 0 || 
        strlen(admin_config->email) == 0) {
        return false;
    }
    
    // Validar valores numéricos
    if (admin_config->session_timeout <= 0 || 
        admin_config->max_login_attempts <= 0) {
        return false;
    }
    
    return true;
}

// Imprimir la configuración actual (para depuración)
void print_config(Config* config) {
    printf("=== Configuración del Sistema ===\n");
    printf("App Name: %s\n", config->app_name);
    printf("Version: %s\n", config->version);
    printf("DB Path: %s\n", config->db_path);
    printf("DB Backup Path: %s\n", config->db_backup_path);
    printf("Log Path: %s\n", config->log_path);
    printf("Log Level: %s\n", config->log_level);
    printf("Max Menu Items: %d\n", config->max_menu_items);
    printf("Clear Screen: %s\n", config->clear_screen ? "true" : "false");
    printf("==============================\n");
}

// Imprimir la configuración de administrador (para depuración)
void print_admin_config(AdminConfig* admin_config) {
    printf("=== Configuración de Administrador ===\n");
    printf("Username: %s\n", admin_config->username);
    printf("Password: %s\n", admin_config->password);
    printf("Email: %s\n", admin_config->email);
    printf("Session Timeout: %d minutos\n", admin_config->session_timeout);
    printf("Max Login Attempts: %d\n", admin_config->max_login_attempts);
    printf("====================================\n");
}

// Guardar cambios en la configuración (opcional para Hito 2)
bool save_config(const char* config_path, Config* config) {
    FILE* file = fopen(config_path, "w");
    if (!file) {
        fprintf(stderr, "Error: No se pudo abrir el archivo para guardar la configuración: %s\n", config_path);
        return false;
    }
    
    // Escribir sección general
    fprintf(file, "[general]\n");
    fprintf(file, "app_name=%s\n", config->app_name);
    fprintf(file, "version=%s\n\n", config->version);
    
    // Escribir sección de base de datos
    fprintf(file, "[database]\n");
    fprintf(file, "db_path=%s\n", config->db_path);
    fprintf(file, "db_backup_path=%s\n\n", config->db_backup_path);
    
    // Escribir sección de logs
    fprintf(file, "[logs]\n");
    fprintf(file, "log_path=%s\n", config->log_path);
    fprintf(file, "log_level=%s\n\n", config->log_level);
    
    // Escribir sección de UI
    fprintf(file, "[ui]\n");
    fprintf(file, "max_menu_items=%d\n", config->max_menu_items);
    fprintf(file, "clear_screen=%s\n", config->clear_screen ? "true" : "false");
    
    fclose(file);
    return true;
}

// Guardar cambios en la configuración de administrador (opcional para Hito 2)
bool save_admin_config(const char* admin_config_path, AdminConfig* admin_config) {
    FILE* file = fopen(admin_config_path, "w");
    if (!file) {
        fprintf(stderr, "Error: No se pudo abrir el archivo para guardar la configuración de administrador: %s\n", admin_config_path);
        return false;
    }
    
    // Escribir sección admin
    fprintf(file, "[admin]\n");
    fprintf(file, "username=%s\n", admin_config->username);
    fprintf(file, "password=%s\n", admin_config->password);
    fprintf(file, "email=%s\n\n", admin_config->email);
    
    // Escribir sección de seguridad
    fprintf(file, "[security]\n");
    fprintf(file, "session_timeout=%d\n", admin_config->session_timeout);
    fprintf(file, "max_login_attempts=%d\n", admin_config->max_login_attempts);
    
    fclose(file);
    return true;
}