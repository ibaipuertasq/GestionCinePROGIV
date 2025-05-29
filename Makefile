# Makefile único para el proyecto Cliente-Servidor
# Compatible con Windows usando MinGW/MSYS2

# Compiladores
CC = gcc
CXX = g++

# Flags de compilación
CFLAGS = -Wall -Wextra -std=c11 -g -D_WIN32_WINNT=0x0601
CXXFLAGS = -Wall -Wextra -std=c++17 -g -D_WIN32_WINNT=0x0601

# Directorios
LIBDIR = lib
MODELDIR = models
UTILDIR = utils

# Librerías para Windows
LIBS = -lws2_32 -lm
SQLITE_SOURCES = $(LIBDIR)/sqlite3.c

# Archivos fuente del servidor
SERVER_SOURCES = servidor.c \
                 config.c \
                 auth.c \
                 test_data.c \
                 $(MODELDIR)/usuario.c \
                 $(MODELDIR)/pelicula.c \
                 $(MODELDIR)/sala.c \
                 $(MODELDIR)/sesion.c \
                 $(MODELDIR)/venta.c \
                 $(MODELDIR)/billete.c \
                 $(MODELDIR)/asiento.c \
                 $(UTILDIR)/logger.c \
                 $(UTILDIR)/memory.c \
                 $(UTILDIR)/database.c \
                 $(SQLITE_SOURCES)

# Archivos fuente del cliente
CLIENT_SOURCES = cliente.cpp

# Ejecutables
SERVIDOR_EXE = servidor.exe
CLIENTE_EXE = cliente.exe

# Includes
INCLUDES = -I$(LIBDIR) -I$(MODELDIR) -I$(UTILDIR) -I.

# Targets
.PHONY: all clean run-server run-client help check test

# Target principal
all: $(SERVIDOR_EXE) $(CLIENTE_EXE)
	@echo.
	@echo ✅ Compilacion completada exitosamente
	@echo.
	@echo Para ejecutar:
	@echo   Servidor: make run-server
	@echo   Cliente:  make run-client
	@echo.

# Compilar servidor
$(SERVIDOR_EXE): $(SERVER_SOURCES)
	@echo 🔧 Compilando servidor...
	$(CC) $(CFLAGS) $(INCLUDES) -o $(SERVIDOR_EXE) $(SERVER_SOURCES) $(LIBS)
	@echo ✅ Servidor compilado: $(SERVIDOR_EXE)

# Compilar cliente
$(CLIENTE_EXE): $(CLIENT_SOURCES)
	@echo 🔧 Compilando cliente...
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $(CLIENTE_EXE) $(CLIENT_SOURCES) $(LIBS)
	@echo ✅ Cliente compilado: $(CLIENTE_EXE)

# Limpiar archivos compilados
clean:
	@echo 🧹 Limpiando archivos compilados...
	@if exist $(SERVIDOR_EXE) del $(SERVIDOR_EXE)
	@if exist $(CLIENTE_EXE) del $(CLIENTE_EXE)
	@if exist *.o del *.o
	@echo ✅ Limpieza completada

# Ejecutar servidor
run-server: $(SERVIDOR_EXE)
	@echo 🚀 Iniciando servidor en puerto 5000...
	@echo Para detener: Ctrl+C
	@echo.
	./$(SERVIDOR_EXE)

# Ejecutar cliente
run-client: $(CLIENTE_EXE)
	@echo 🚀 Iniciando cliente...
	@echo Conectando a localhost:5000
	@echo.
	./$(CLIENTE_EXE)

# Verificar archivos necesarios
check:
	@echo 🔍 Verificando estructura del proyecto...
	@if not exist config\config.ini (echo ❌ config\config.ini no encontrado && exit /b 1)
	@if not exist database\cine.db (echo ❌ database\cine.db no encontrada && exit /b 1)
	@if not exist $(LIBDIR)\sqlite3.c (echo ❌ $(LIBDIR)\sqlite3.c no encontrado && exit /b 1)
	@if not exist $(MODELDIR)\usuario.c (echo ❌ $(MODELDIR)\usuario.c no encontrado && exit /b 1)
	@echo ✅ Todos los archivos necesarios están presentes

# Test de compilación
test: clean all
	@echo 🧪 Ejecutando test de compilación...
	@echo ✅ Test completado exitosamente

# Compilar solo servidor
servidor: $(SERVIDOR_EXE)

# Compilar solo cliente
cliente: $(CLIENTE_EXE)

# Ayuda
help:
	@echo.
	@echo 📋 Comandos disponibles:
	@echo.
	@echo   make              - Compilar servidor y cliente
	@echo   make servidor     - Compilar solo el servidor
	@echo   make cliente      - Compilar solo el cliente
	@echo   make run-server   - Ejecutar servidor
	@echo   make run-client   - Ejecutar cliente
	@echo   make clean        - Limpiar archivos compilados
	@echo   make check        - Verificar archivos del proyecto
	@echo   make test         - Test de compilación completo
	@echo   make help         - Mostrar esta ayuda
	@echo.
	@echo 🎯 Flujo de trabajo tipico:
	@echo   1. make check
	@echo   2. make
	@echo   3. make run-server (en una terminal)
	@echo   4. make run-client (en otra terminal)
	@echo.

# Reglas para compilación incremental (opcional)
%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@