#include "logger.h"
#include "../config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>

// Archivo de log
static FILE* log_file = NULL;

// Nivel mínimo de log
static LogLevel min_log_level = LOG_INFO;

// Inicializar el sistema de logging
bool log_init(const char* log_path, LogLevel min_level) {
    // Cerrar el archivo si ya está abierto
    if (log_file) {
        fclose(log_file);
    }
    
    // Abrir el archivo de log en modo append
    log_file = fopen(log_path, "a");
    if (!log_file) {
        fprintf(stderr, "Error: No se pudo abrir el archivo de log: %s\n", log_path);
        return false;
    }
    
    min_log_level = min_level;
    
    // Registrar inicio del sistema
    log_info("Sistema de logging inicializado");
    
    return true;
}

// Cerrar el sistema de logging
void log_close() {
    if (log_file) {
        // Registrar cierre del sistema
        log_info("Sistema de logging cerrado");
        
        fclose(log_file);
        log_file = NULL;
    }
}

// Función interna para obtener la hora actual formateada
static void get_current_time(char* buffer, size_t size) {
    time_t now = time(NULL);
    struct tm* tm_info = localtime(&now);
    strftime(buffer, size, "%Y-%m-%d %H:%M:%S", tm_info);
}

// Registrar un mensaje con nivel personalizado
void log_message(LogLevel level, const char* format, ...) {
    // Verificar si el nivel de log es suficiente
    if (level < min_log_level || !log_file) {
        return;
    }
    
    // Obtener la hora actual
    char time_buffer[20];
    get_current_time(time_buffer, sizeof(time_buffer));
    
    // Obtener el nivel como cadena
    const char* level_str = log_level_to_string(level);
    
    // Escribir el encabezado del mensaje
    fprintf(log_file, "[%s] [%s] ", time_buffer, level_str);
    
    // Escribir el mensaje formateado
    va_list args;
    va_start(args, format);
    vfprintf(log_file, format, args);
    va_end(args);
    
    // Añadir salto de línea
    fprintf(log_file, "\n");
    
    // Forzar escritura inmediata
    fflush(log_file);
}

// Funciones para los diferentes niveles de log
void log_debug(const char* format, ...) {
    if (LOG_DEBUG < min_log_level || !log_file) {
        return;
    }
    
    char time_buffer[20];
    get_current_time(time_buffer, sizeof(time_buffer));
    
    fprintf(log_file, "[%s] [DEBUG] ", time_buffer);
    
    va_list args;
    va_start(args, format);
    vfprintf(log_file, format, args);
    va_end(args);
    
    fprintf(log_file, "\n");
    fflush(log_file);
}

void log_info(const char* format, ...) {
    if (LOG_INFO < min_log_level || !log_file) {
        return;
    }
    
    char time_buffer[20];
    get_current_time(time_buffer, sizeof(time_buffer));
    
    fprintf(log_file, "[%s] [INFO] ", time_buffer);
    
    va_list args;
    va_start(args, format);
    vfprintf(log_file, format, args);
    va_end(args);
    
    fprintf(log_file, "\n");
    fflush(log_file);
}

void log_warning(const char* format, ...) {
    if (LOG_WARNING < min_log_level || !log_file) {
        return;
    }
    
    char time_buffer[20];
    get_current_time(time_buffer, sizeof(time_buffer));
    
    fprintf(log_file, "[%s] [WARNING] ", time_buffer);
    
    va_list args;
    va_start(args, format);
    vfprintf(log_file, format, args);
    va_end(args);
    
    fprintf(log_file, "\n");
    fflush(log_file);
}

void log_error(const char* format, ...) {
    if (LOG_ERROR < min_log_level || !log_file) {
        return;
    }
    
    char time_buffer[20];
    get_current_time(time_buffer, sizeof(time_buffer));
    
    fprintf(log_file, "[%s] [ERROR] ", time_buffer);
    
    va_list args;
    va_start(args, format);
    vfprintf(log_file, format, args);
    va_end(args);
    
    fprintf(log_file, "\n");
    fflush(log_file);
}

void log_critical(const char* format, ...) {
    if (LOG_CRITICAL < min_log_level || !log_file) {
        return;
    }
    
    char time_buffer[20];
    get_current_time(time_buffer, sizeof(time_buffer));
    
    fprintf(log_file, "[%s] [CRITICAL] ", time_buffer);
    
    va_list args;
    va_start(args, format);
    vfprintf(log_file, format, args);
    va_end(args);
    
    fprintf(log_file, "\n");
    fflush(log_file);
}

// Convertir cadena de nivel de log a enum
LogLevel log_level_from_string(const char* level_str) {
    if (!level_str) {
        return LOG_INFO; // Valor por defecto
    }
    
    if (strcmp(level_str, "DEBUG") == 0) {
        return LOG_DEBUG;
    } else if (strcmp(level_str, "INFO") == 0) {
        return LOG_INFO;
    } else if (strcmp(level_str, "WARNING") == 0) {
        return LOG_WARNING;
    } else if (strcmp(level_str, "ERROR") == 0) {
        return LOG_ERROR;
    } else if (strcmp(level_str, "CRITICAL") == 0) {
        return LOG_CRITICAL;
    }
    
    return LOG_INFO; // Valor por defecto
}

// Convertir enum de nivel de log a cadena
const char* log_level_to_string(LogLevel level) {
    switch (level) {
        case LOG_DEBUG:
            return "DEBUG";
        case LOG_INFO:
            return "INFO";
        case LOG_WARNING:
            return "WARNING";
        case LOG_ERROR:
            return "ERROR";
        case LOG_CRITICAL:
            return "CRITICAL";
        default:
            return "UNKNOWN";
    }
}