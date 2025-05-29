#ifndef LOGGER_H
#define LOGGER_H

#include <stdbool.h>

// Niveles de log
typedef enum {
    LOG_DEBUG,
    LOG_INFO,
    LOG_WARNING,
    LOG_ERROR,
    LOG_CRITICAL
} LogLevel;

// Inicializar el sistema de logging
bool log_init(const char* log_path, LogLevel min_level);

// Cerrar el sistema de logging
void log_close();

// Funciones para registrar eventos
void log_debug(const char* format, ...);
void log_info(const char* format, ...);
void log_warning(const char* format, ...);
void log_error(const char* format, ...);
void log_critical(const char* format, ...);

// Registrar un mensaje con nivel personalizado
void log_message(LogLevel level, const char* format, ...);

// Convertir cadena de nivel de log a enum
LogLevel log_level_from_string(const char* level_str);

// Convertir enum de nivel de log a cadena
const char* log_level_to_string(LogLevel level);

#endif // LOGGER_H