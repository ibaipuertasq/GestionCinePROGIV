# Makefile para CineGestion - Hito 2

# Compilador y flags
CC = gcc
CFLAGS = -Wall -Wextra -g -I.
LDFLAGS = -lsqlite3

# Directorios
SRC_DIR = src
BUILD_DIR = build
BIN_DIR = bin

# Archivos fuente
SRCS = $(SRC_DIR)/main.c \
       $(SRC_DIR)/config.c \
       $(SRC_DIR)/database.c \
       $(SRC_DIR)/auth.c \
       $(SRC_DIR)/menu.c \
       $(SRC_DIR)/utils/logger.c \
       $(SRC_DIR)/utils/memory.c \
       $(SRC_DIR)/models/usuario.c \
       $(SRC_DIR)/models/pelicula.c \
       $(SRC_DIR)/models/sala.c \
       $(SRC_DIR)/models/asiento.c \
       $(SRC_DIR)/models/sesion.c \
       $(SRC_DIR)/models/billete.c \
       $(SRC_DIR)/models/venta.c

# Archivos objeto
OBJS = $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRCS))

# Directorio de salida para archivos objeto
OBJDIRS = $(sort $(dir $(OBJS)))

# Nombre del ejecutable
TARGET = $(BIN_DIR)/cinegestion

# Regla principal
all: directories $(TARGET)

# Crear directorios necesarios
directories:
	@mkdir -p $(OBJDIRS)
	@mkdir -p $(BIN_DIR)
	@mkdir -p config
	@mkdir -p data
	@mkdir -p logs

# Regla para crear el ejecutable
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)
	@echo "Compilación finalizada. Ejecutable creado en $(TARGET)"

# Regla para compilar archivos fuente a objetos
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# Regla para limpiar archivos temporales
clean:
	rm -rf $(BUILD_DIR)
	@echo "Archivos temporales eliminados"

# Regla para limpiar todo (incluyendo ejecutable)
distclean: clean
	rm -rf $(BIN_DIR)
	@echo "Directorio bin eliminado"

# Regla para ejecutar el programa
run: all
	$(TARGET)

# Evitar que make confunda estos objetivos con archivos existentes
.PHONY: all directories clean distclean run