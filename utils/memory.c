#include "memory.h"
#include "logger.h"
#include <stdio.h>
#include <string.h>

// Estructura para rastrear la memoria asignada
struct MemoryBlock {
    void* ptr;              // Puntero a la memoria asignada
    size_t size;            // Tamaño de la memoria asignada
    const char* file;       // Archivo donde se asignó
    int line;               // Línea donde se asignó
    MemoryBlock* next;      // Siguiente bloque en la lista
};

// Lista enlazada de bloques de memoria
static MemoryBlock* memory_blocks = NULL;

// Contador de bloques
static int block_count = 0;

// Memoria total asignada
static size_t total_allocated = 0;

// Inicializar el sistema de gestión de memoria
void memory_init() {
    memory_blocks = NULL;
    block_count = 0;
    total_allocated = 0;
    log_debug("Sistema de gestión de memoria inicializado");
}

// Liberar todos los recursos del sistema de gestión de memoria
void memory_cleanup() {
    // Verificar si hay fugas de memoria
    if (block_count > 0) {
        log_warning("Se detectaron %d bloques de memoria sin liberar", block_count);
        memory_leaks_report();
    } else {
        log_debug("No se detectaron fugas de memoria");
    }
    
    // Liberar todos los bloques de memoria
    MemoryBlock* current = memory_blocks;
    while (current) {
        MemoryBlock* next = current->next;
        free(current->ptr);
        free(current);
        current = next;
    }
    
    memory_blocks = NULL;
    block_count = 0;
    total_allocated = 0;
    
    log_debug("Sistema de gestión de memoria liberado");
}

// Añadir un bloque a la lista de bloques
static void add_block(void* ptr, size_t size, const char* file, int line) {
    MemoryBlock* block = (MemoryBlock*)malloc(sizeof(MemoryBlock));
    if (!block) {
        log_critical("No se pudo asignar memoria para el bloque de seguimiento");
        return;
    }
    
    block->ptr = ptr;
    block->size = size;
    block->file = file;
    block->line = line;
    block->next = memory_blocks;
    
    memory_blocks = block;
    block_count++;
    total_allocated += size;
    
    log_debug("Memoria asignada: %zu bytes en %s:%d", size, file, line);
}

// Eliminar un bloque de la lista de bloques
static void remove_block(void* ptr) {
    MemoryBlock** current = &memory_blocks;
    
    while (*current) {
        if ((*current)->ptr == ptr) {
            MemoryBlock* to_remove = *current;
            *current = to_remove->next;
            
            total_allocated -= to_remove->size;
            block_count--;
            
            log_debug("Memoria liberada: %zu bytes desde %s:%d", 
                     to_remove->size, to_remove->file, to_remove->line);
            
            free(to_remove);
            return;
        }
        
        current = &((*current)->next);
    }
    
    log_error("Intento de liberar memoria no asignada: %p", ptr);
}

// Encontrar un bloque en la lista
static MemoryBlock* find_block(void* ptr) {
    MemoryBlock* current = memory_blocks;
    
    while (current) {
        if (current->ptr == ptr) {
            return current;
        }
        
        current = current->next;
    }
    
    return NULL;
}

// Asignar memoria con seguimiento
void* memory_alloc(size_t size, const char* file, int line) {
    void* ptr = malloc(size);
    
    if (!ptr) {
        log_critical("Fallo al asignar %zu bytes en %s:%d", size, file, line);
        return NULL;
    }
    
    add_block(ptr, size, file, line);
    return ptr;
}

// Liberar memoria con seguimiento
void memory_free(void* ptr) {
    if (!ptr) {
        log_warning("Intento de liberar un puntero NULL");
        return;
    }
    
    remove_block(ptr);
    free(ptr);
}

// Reasignar memoria con seguimiento
void* memory_realloc(void* ptr, size_t size, const char* file, int line) {
    if (!ptr) {
        return memory_alloc(size, file, line);
    }
    
    MemoryBlock* block = find_block(ptr);
    if (!block) {
        log_error("Intento de reasignar memoria no asignada: %p en %s:%d", ptr, file, line);
        return NULL;
    }
    
    void* new_ptr = realloc(ptr, size);
    if (!new_ptr) {
        log_critical("Fallo al reasignar %zu bytes en %s:%d", size, file, line);
        return NULL;
    }
    
    // Actualizar o reemplazar el bloque
    if (new_ptr == ptr) {
        // Solo cambió el tamaño
        total_allocated = total_allocated - block->size + size;
        block->size = size;
        log_debug("Memoria reasignada (mismo puntero): %zu bytes en %s:%d", size, file, line);
    } else {
        // Cambió el puntero, eliminar el bloque antiguo y añadir uno nuevo
        remove_block(ptr);
        add_block(new_ptr, size, file, line);
        log_debug("Memoria reasignada (nuevo puntero): %zu bytes en %s:%d", size, file, line);
    }
    
    return new_ptr;
}

// Obtener el número actual de bloques asignados
int memory_block_count() {
    return block_count;
}

// Obtener el total de memoria asignada actualmente
size_t memory_total_allocated() {
    return total_allocated;
}

// Imprimir informe de memoria actual
void memory_report() {
    printf("===== Informe de Memoria =====\n");
    printf("Bloques asignados: %d\n", block_count);
    printf("Memoria total: %zu bytes\n", total_allocated);
    printf("=============================\n");
}

// Imprimir fugas de memoria (bloques que no han sido liberados)
void memory_leaks_report() {
    if (block_count == 0) {
        printf("No se detectaron fugas de memoria.\n");
        return;
    }
    
    printf("===== Informe de Fugas de Memoria =====\n");
    printf("Se encontraron %d bloques sin liberar:\n", block_count);
    
    int i = 1;
    MemoryBlock* current = memory_blocks;
    
    while (current) {
        printf("%d. %zu bytes en %s:%d (ptr: %p)\n", 
               i++, current->size, current->file, current->line, current->ptr);
        
        current = current->next;
    }
    
    printf("Total: %zu bytes\n", total_allocated);
    printf("=====================================\n");
}