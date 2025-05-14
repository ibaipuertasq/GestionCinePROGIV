#ifndef MEMORY_H
#define MEMORY_H

#include <stdlib.h>
#include <stdbool.h>

// Estructura para rastrear la memoria asignada
typedef struct MemoryBlock MemoryBlock;

// Inicializar el sistema de gestión de memoria
void memory_init();

// Liberar todos los recursos del sistema de gestión de memoria
void memory_cleanup();

// Asignar memoria con seguimiento
void* memory_alloc(size_t size, const char* file, int line);

// Liberar memoria con seguimiento
void memory_free(void* ptr);

// Reasignar memoria con seguimiento
void* memory_realloc(void* ptr, size_t size, const char* file, int line);

// Obtener el número actual de bloques asignados
int memory_block_count();

// Obtener el total de memoria asignada actualmente
size_t memory_total_allocated();

// Imprimir informe de memoria actual
void memory_report();

// Imprimir fugas de memoria (bloques que no han sido liberados)
void memory_leaks_report();

// Macros para facilitar el uso
#define MEM_ALLOC(size) memory_alloc(size, __FILE__, __LINE__)
#define MEM_FREE(ptr) memory_free(ptr)
#define MEM_REALLOC(ptr, size) memory_realloc(ptr, size, __FILE__, __LINE__)

#endif // MEMORY_H