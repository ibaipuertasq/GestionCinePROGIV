#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// Includes específicos para Windows
#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
    #define close closesocket
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
#endif

// Incluir TODO tu código existente (sin cambios)
#include "config.h"
#include "auth.h"
#include "models/usuario.h"
#include "models/pelicula.h"
#include "models/sala.h"
#include "models/sesion.h"
#include "models/venta.h"
#include "models/billete.h"
#include "models/asiento.h"
#include "utils/logger.h"
#include "utils/memory.h"
#include "utils/database.h"
#include "test_data.h"

#define PORT 5000
#define MAX_BUFFER 4096

// Función que procesa comandos usando TUS funciones existentes
char* procesar_comando(char* comando) {
    char* respuesta = malloc(MAX_BUFFER);
    memset(respuesta, 0, MAX_BUFFER);
    
    // Dividir comando por ':'
    char* cmd = strtok(comando, ":");
    
    if (strcmp(cmd, "LOGIN") == 0) {
        char* email = strtok(NULL, ":");
        char* password = strtok(NULL, ":");
        
        if (auth_login(email, password)) {
            Usuario* user = auth_obtener_usuario_actual();
            // Formato mejorado: OK:id:nombre:tipo:correo:telefono:saldo
            snprintf(respuesta, MAX_BUFFER, "OK:%d:%s:%s:%s:%s:%.2f", 
                    user->id,
                    user->nombre, 
                    user->tipo == USUARIO_ADMINISTRADOR ? "admin" : "cliente",
                    user->correo,
                    user->telefono ? user->telefono : "",
                    0.0);  // saldo por defecto
            log_info("Usuario %s logueado correctamente", user->nombre);
        } else {
            strcpy(respuesta, "ERROR:Credenciales incorrectas");
            log_warning("Intento de login fallido para %s", email);
        }
    }
    else if (strcmp(cmd, "LOGOUT") == 0) {
        auth_logout();
        strcpy(respuesta, "OK:Sesión cerrada");
        log_info("Usuario deslogueado");
    }
    else if (strcmp(cmd, "GET_MOVIES") == 0) {
        Pelicula* peliculas = NULL;
        int num_peliculas = 0;
        
        if (pelicula_listar(&peliculas, &num_peliculas)) {
            strcpy(respuesta, "OK:");
            for (int i = 0; i < num_peliculas; i++) {
                char temp[300];
                snprintf(temp, sizeof(temp), "%d|%s|%d|%s;", 
                        peliculas[i].id, peliculas[i].titulo, 
                        peliculas[i].duracion, peliculas[i].genero);
                strcat(respuesta, temp);
            }
            pelicula_liberar_lista(peliculas, num_peliculas);
            log_info("Enviadas %d películas al cliente", num_peliculas);
        } else {
            strcpy(respuesta, "ERROR:No se pudieron obtener películas");
            log_error("Error obteniendo lista de películas");
        }
    }
    else if (strcmp(cmd, "CREATE_MOVIE") == 0) {
        if (!auth_sesion_activa() || !auth_es_administrador()) {
            strcpy(respuesta, "ERROR:Acceso denegado - Requiere permisos de administrador");
            log_warning("Intento de crear película sin permisos");
            return respuesta;
        }
        
        char* titulo = strtok(NULL, ":");
        char* duracion_str = strtok(NULL, ":");
        char* genero = strtok(NULL, ":");
        
        if (!titulo || !duracion_str || !genero) {
            strcpy(respuesta, "ERROR:Parámetros incompletos");
            return respuesta;
        }
        
        Pelicula nueva_pelicula = {0};
        strncpy(nueva_pelicula.titulo, titulo, sizeof(nueva_pelicula.titulo)-1);
        nueva_pelicula.duracion = atoi(duracion_str);
        strncpy(nueva_pelicula.genero, genero, sizeof(nueva_pelicula.genero)-1);
        
        if (pelicula_crear(&nueva_pelicula)) {
            snprintf(respuesta, MAX_BUFFER, "OK:%d", nueva_pelicula.id);
            log_info("Película '%s' creada con ID %d", nueva_pelicula.titulo, nueva_pelicula.id);
        } else {
            strcpy(respuesta, "ERROR:No se pudo crear la película");
            log_error("Error creando película '%s'", titulo);
        }
    }
    else if (strcmp(cmd, "GET_ROOMS") == 0) {
        Sala* salas = NULL;
        int num_salas = 0;
        
        if (sala_listar(&salas, &num_salas)) {
            strcpy(respuesta, "OK:");
            for (int i = 0; i < num_salas; i++) {
                char temp[100];
                int asientos_libres = sala_contar_asientos_libres(salas[i].id);
                snprintf(temp, sizeof(temp), "%d|%d|%d;", 
                        salas[i].id, salas[i].numero_asientos, asientos_libres);
                strcat(respuesta, temp);
            }
            sala_liberar_lista(salas, num_salas);
            log_info("Enviadas %d salas al cliente", num_salas);
        } else {
            strcpy(respuesta, "ERROR:No se pudieron obtener salas");
            log_error("Error obteniendo lista de salas");
        }
    }
    else if (strcmp(cmd, "CREATE_ROOM") == 0) {
        if (!auth_sesion_activa() || !auth_es_administrador()) {
            strcpy(respuesta, "ERROR:Acceso denegado - Requiere permisos de administrador");
            log_warning("Intento de crear sala sin permisos");
            return respuesta;
        }
        
        char* num_asientos_str = strtok(NULL, ":");
        
        if (!num_asientos_str) {
            strcpy(respuesta, "ERROR:Número de asientos requerido");
            return respuesta;
        }
        
        Sala nueva_sala = {0};
        nueva_sala.numero_asientos = atoi(num_asientos_str);
        
        if (nueva_sala.numero_asientos <= 0 || nueva_sala.numero_asientos > 1000) {
            strcpy(respuesta, "ERROR:Número de asientos inválido (1-1000)");
            return respuesta;
        }
        
        if (sala_crear(&nueva_sala)) {
            snprintf(respuesta, MAX_BUFFER, "OK:%d", nueva_sala.id);
            log_info("Sala creada con ID %d y %d asientos", nueva_sala.id, nueva_sala.numero_asientos);
        } else {
            strcpy(respuesta, "ERROR:No se pudo crear la sala");
            log_error("Error creando sala con %d asientos", nueva_sala.numero_asientos);
        }
    }
    else if (strcmp(cmd, "GET_SESSIONS") == 0) {
        Sesion* sesiones = NULL;
        int num_sesiones = 0;
        
        if (sesion_listar(&sesiones, &num_sesiones)) {
            strcpy(respuesta, "OK:");
            for (int i = 0; i < num_sesiones; i++) {
                char temp[300];
                snprintf(temp, sizeof(temp), "%d|%d|%d|%s|%s;", 
                        sesiones[i].id, sesiones[i].pelicula_id,
                        sesiones[i].sala_id, sesiones[i].hora_inicio,
                        sesiones[i].hora_fin);
                strcat(respuesta, temp);
            }
            sesion_liberar_lista(sesiones, num_sesiones);
            log_info("Enviadas %d sesiones al cliente", num_sesiones);
        } else {
            strcpy(respuesta, "ERROR:No se pudieron obtener sesiones");
            log_error("Error obteniendo lista de sesiones");
        }
    }
    else if (strcmp(cmd, "GET_SESSIONS_BY_MOVIE") == 0) {
        char* movie_id_str = strtok(NULL, ":");
        
        if (!movie_id_str) {
            strcpy(respuesta, "ERROR:ID de película requerido");
            return respuesta;
        }
        
        int movie_id = atoi(movie_id_str);
        Sesion* sesiones = NULL;
        int num_sesiones = 0;
        
        if (sesion_buscar_por_pelicula(movie_id, &sesiones, &num_sesiones)) {
            strcpy(respuesta, "OK:");
            for (int i = 0; i < num_sesiones; i++) {
                char temp[300];
                snprintf(temp, sizeof(temp), "%d|%d|%d|%s|%s;", 
                        sesiones[i].id, sesiones[i].pelicula_id,
                        sesiones[i].sala_id, sesiones[i].hora_inicio,
                        sesiones[i].hora_fin);
                strcat(respuesta, temp);
            }
            sesion_liberar_lista(sesiones, num_sesiones);
            log_info("Enviadas %d sesiones para película %d", num_sesiones, movie_id);
        } else {
            strcpy(respuesta, "ERROR:No se encontraron sesiones para esta película");
            log_info("No hay sesiones para película %d", movie_id);
        }
    }
    else if (strcmp(cmd, "GET_USER_PURCHASES") == 0) {
        if (!auth_sesion_activa()) {
            strcpy(respuesta, "ERROR:Debe iniciar sesión");
            return respuesta;
        }
        
        char* user_id_str = strtok(NULL, ":");
        int user_id = user_id_str ? atoi(user_id_str) : auth_obtener_usuario_actual()->id;
        
        // Verificar que el usuario puede ver estas compras
        Usuario* current_user = auth_obtener_usuario_actual();
        if (current_user->tipo != USUARIO_ADMINISTRADOR && current_user->id != user_id) {
            strcpy(respuesta, "ERROR:No puede ver compras de otros usuarios");
            log_warning("Usuario %d intentó ver compras de usuario %d", current_user->id, user_id);
            return respuesta;
        }
        
        Venta* ventas = NULL;
        int num_ventas = 0;
        
        if (venta_listar_por_usuario(user_id, &ventas, &num_ventas)) {
            strcpy(respuesta, "OK:");
            for (int i = 0; i < num_ventas; i++) {
                char temp[200];
                snprintf(temp, sizeof(temp), "%d|%s|%.2f;", 
                        ventas[i].id, ventas[i].fecha, ventas[i].precio_total);
                strcat(respuesta, temp);
            }
            venta_liberar_lista(ventas, num_ventas);
            log_info("Enviadas %d compras para usuario %d", num_ventas, user_id);
        } else {
            strcpy(respuesta, "ERROR:No se encontraron compras");
            log_info("No hay compras para usuario %d", user_id);
        }
    }
    else if (strcmp(cmd, "QUIT") == 0) {
        strcpy(respuesta, "BYE");
        log_info("Cliente solicita desconexión");
    }
    else {
        snprintf(respuesta, MAX_BUFFER, "ERROR:Comando no reconocido: %s", cmd);
        log_warning("Comando no reconocido: %s", cmd);
    }
    
    return respuesta;
}

int main() {
    // Inicializar Winsock en Windows
    #ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2,2), &wsaData) != 0) {
        fprintf(stderr, "Error inicializando Winsock\n");
        return -1;
    }
    #endif
    printf("=== SERVIDOR DE GESTIÓN DE CINE ===\n");
    
    // Inicializar tu sistema completo (sin cambios)
    memory_init();
    
    Config config;
    if (!load_config("config/config.ini", &config)) {
        fprintf(stderr, "Error: No se pudo cargar config/config.ini\n");
        return -1;
    }
    
    AdminConfig admin_config;
    if (!load_admin_config("config/admin.ini", &admin_config)) {
        fprintf(stderr, "Error: No se pudo cargar config/admin.ini\n");
        return -1;
    }
    
    LogLevel log_level = log_level_from_string(config.log_level);
    if (!log_init(config.log_path, log_level)) {
        fprintf(stderr, "Error: No se pudo inicializar logging\n");
        return -1;
    }
    
    log_info("===== INICIANDO SERVIDOR DE CINE =====");
    printf("Inicializando base de datos...\n");
    
    if (!db_init(config.db_path)) {
        log_critical("Error inicializando base de datos en %s", config.db_path);
        fprintf(stderr, "Error: No se pudo inicializar la base de datos\n");
        return -1;
    }
    
    printf("Inicializando datos de prueba...\n");
    if (!test_data_init()) {
        log_warning("No se pudieron inicializar datos de prueba");
    }
    
    auth_init();
    log_info("Sistema inicializado correctamente");
    
    // Código del servidor usando las plantillas proporcionadas
    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    char buffer[MAX_BUFFER] = {0};

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Fallo en socket");
        log_critical("Error creando socket");
        return -1;
    }

    // Permitir reutilizar el puerto
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("setsockopt");
        log_error("Error configurando socket");
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Fallo en bind");
        log_critical("Error haciendo bind en puerto %d", PORT);
        return -1;
    }

    if (listen(server_fd, 3) < 0) {
        perror("Fallo en listen");
        log_critical("Error poniendo socket en modo listen");
        return -1;
    }

    printf("✅ Servidor escuchando en puerto %d\n", PORT);
    printf("Esperando conexiones de clientes...\n\n");
    log_info("Servidor escuchando en puerto %d", PORT);

    while (1) {
        printf("⏳ Esperando cliente...\n");
        
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, 
                                (socklen_t*)&addrlen)) < 0) {
            perror("Fallo en accept");
            log_error("Error aceptando conexión");
            continue;
        }
        
        printf("🔗 Cliente conectado\n");
        log_info("Cliente conectado desde %s", inet_ntoa(address.sin_addr));

        // Leer comando del cliente usando recv()
        memset(buffer, 0, MAX_BUFFER);
        int bytes_received = recv(new_socket, buffer, MAX_BUFFER - 1, 0);
        
        if (bytes_received > 0) {
            buffer[bytes_received] = '\0';
            printf("📨 Comando recibido: %s\n", buffer);

            // Procesar comando usando TUS funciones existentes
            char* respuesta = procesar_comando(buffer);
            
            // Enviar respuesta usando send()
            send(new_socket, respuesta, strlen(respuesta), 0);
            printf("📤 Respuesta enviada: %.100s%s\n", 
                   respuesta, strlen(respuesta) > 100 ? "..." : "");

            free(respuesta);
        } else {
            printf("❌ Error recibiendo datos del cliente\n");
            log_error("Error recibiendo datos del cliente");
        }

        close(new_socket);
        printf("🔌 Cliente desconectado\n\n");

        // Si es QUIT, terminar servidor
        if (strcmp(buffer, "QUIT") == 0) {
            printf("🛑 Comando QUIT recibido, cerrando servidor...\n");
            log_info("Servidor cerrado por comando QUIT");
            break;
        }
    }

    close(server_fd);
    
    // Limpiar tu sistema
    log_info("===== CERRANDO SERVIDOR =====");
    db_close();
    log_close();
    memory_cleanup();
    
    printf("✅ Servidor cerrado correctamente\n");
    #ifdef _WIN32
    WSACleanup();
    #endif
    
    return 0;
}