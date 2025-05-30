#ifndef MEMORY_H
#define MEMORY_H

#include <stddef.h>
#include <stdbool.h>

// Evitar conflicto con Windows
#ifdef _WIN32
    #ifdef MEM_FREE
        #undef MEM_FREE
    #endif
    #ifdef MEM_ALLOC
        #undef MEM_ALLOC
    #endif
#endif

// Estructura para estadísticas de memoria
typedef struct {
    size_t total_allocations;
    size_t total_deallocations;
    size_t current_allocations;
    size_t peak_allocations;
    size_t total_bytes_allocated;
    size_t current_bytes_allocated;
    size_t peak_bytes_allocated;
} MemoryStats;

// Funciones de gestión de memoria
bool memory_init(void);
void memory_cleanup(void);
void* memory_alloc(size_t size, const char* file, int line);
void memory_free(void* ptr);
void* memory_realloc(void* ptr, size_t size, const char* file, int line);
void memory_print_stats(void);
bool memory_check_leaks(void);
MemoryStats memory_get_stats(void);

// Función para reportar fugas de memoria
void memory_leaks_report(void);

// Macros para facilitar el uso (redefinidas después del undef)
#define MEM_ALLOC(size) memory_alloc(size, __FILE__, __LINE__)
#define MEM_FREE(ptr) memory_free(ptr)
#define MEM_REALLOC(ptr, size) memory_realloc(ptr, size, __FILE__, __LINE__)

#endif // MEMORY_H