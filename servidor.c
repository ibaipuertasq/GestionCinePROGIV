#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// Includes específicos para Windows
#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #define close closesocket
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
#endif

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

// Función que procesa comandos con DEBUG mejorado
char* procesar_comando(char* comando) {
    char* respuesta = malloc(MAX_BUFFER);
    memset(respuesta, 0, MAX_BUFFER);
    
    printf(" DEBUG - Procesando comando: %s\n", comando);
    
    // Crear una copia del comando para strtok
    char* comando_copia = malloc(strlen(comando) + 1);
    strcpy(comando_copia, comando);
    
    // Dividir comando por ':'
    char* cmd = strtok(comando_copia, ":");
    
    if (strcmp(cmd, "LOGIN") == 0) {
        char* email = strtok(NULL, ":");
        char* password = strtok(NULL, ":");
        
        printf("🔍 DEBUG - Intento de login: %s / %s\n", email, password);
        
        if (auth_login(email, password)) {
            Usuario* user = auth_obtener_usuario_actual();
            snprintf(respuesta, MAX_BUFFER, "OK:%d:%s:%s:%s:%s:%.2f", 
                    user->id,
                    user->nombre, 
                    user->tipo == USUARIO_ADMINISTRADOR ? "admin" : "cliente",
                    user->correo,
                    user->telefono ? user->telefono : "",
                    0.0);
            
            printf(" DEBUG - Login exitoso para: %s (ID: %d)\n", user->nombre, user->id);
            log_info("Usuario %s logueado correctamente", user->nombre);
        } else {
            strcpy(respuesta, "ERROR:Credenciales incorrectas");
            printf(" DEBUG - Login fallido para: %s\n", email);
            log_warning("Intento de login fallido para %s", email);
        }
    }
    else if (strcmp(cmd, "LOGOUT") == 0) {
        auth_logout();
        strcpy(respuesta, "OK:Sesion cerrada");
        printf(" DEBUG - Usuario deslogueado\n");
        log_info("Usuario deslogueado");
    }
    else if (strcmp(cmd, "GET_MOVIES") == 0) {
        printf(" DEBUG - Obteniendo peliculas...\n");
        
        Pelicula* peliculas = NULL;
        int num_peliculas = 0;
        
        if (pelicula_listar(&peliculas, &num_peliculas)) {
            printf(" DEBUG - Encontradas %d peliculas\n", num_peliculas);
            strcpy(respuesta, "OK:");
            
            for (int i = 0; i < num_peliculas; i++) {
                char temp[300];
                snprintf(temp, sizeof(temp), "%d|%s|%d|%s;", 
                        peliculas[i].id, peliculas[i].titulo, 
                        peliculas[i].duracion, peliculas[i].genero);
                strcat(respuesta, temp);
                printf("  - Pelicula %d: %s\n", peliculas[i].id, peliculas[i].titulo);
            }
            
            pelicula_liberar_lista(peliculas, num_peliculas);
            log_info("Enviadas %d peliculas al cliente", num_peliculas);
        } else {
            strcpy(respuesta, "ERROR:No se pudieron obtener peliculas");
            printf("i DEBUG - Error obteniendo peliculas\n");
            log_error("Error obteniendo lista de peliculas");
        }
    }
    else if (strcmp(cmd, "CREATE_MOVIE") == 0) {
        printf(" DEBUG - Creando pelicula...\n");
        
        // Verificar autenticación primero
        if (!auth_sesion_activa()) {
            printf(" DEBUG - No hay sesion activa\n");
            strcpy(respuesta, "ERROR:Debe iniciar sesion");
            free(comando_copia);
            return respuesta;
        }
        
        if (!auth_es_administrador()) {
            printf(" DEBUG - Usuario no es administrador\n");
            strcpy(respuesta, "ERROR:Acceso denegado - Requiere permisos de administrador");
            log_warning("Intento de crear pelicula sin permisos");
            free(comando_copia);
            return respuesta;
        }
        
        char* titulo = strtok(NULL, ":");
        char* duracion_str = strtok(NULL, ":");
        char* genero = strtok(NULL, ":");
        
        printf("🔍 DEBUG - Datos película: título='%s', duración='%s', género='%s'\n", 
               titulo ? titulo : "NULL", 
               duracion_str ? duracion_str : "NULL", 
               genero ? genero : "NULL");
        
        if (!titulo || !duracion_str || !genero) {
            strcpy(respuesta, "ERROR:Parametros incompletos");
            printf(" DEBUG - Parametros incompletos\n");
            free(comando_copia);
            return respuesta;
        }
        
        Pelicula nueva_pelicula = {0};
        strncpy(nueva_pelicula.titulo, titulo, sizeof(nueva_pelicula.titulo)-1);
        nueva_pelicula.duracion = atoi(duracion_str);
        strncpy(nueva_pelicula.genero, genero, sizeof(nueva_pelicula.genero)-1);
        
        if (pelicula_crear(&nueva_pelicula)) {
            snprintf(respuesta, MAX_BUFFER, "OK:%d", nueva_pelicula.id);
            printf(" DEBUG - Pelicula creada con ID %d\n", nueva_pelicula.id);
            log_info("Pelicula '%s' creada con ID %d", nueva_pelicula.titulo, nueva_pelicula.id);
        } else {
            strcpy(respuesta, "ERROR:No se pudo crear la pelicula");
            printf(" DEBUG - Error creando pelicula\n");
            log_error("Error creando pelicula '%s'", titulo);
        }
    }
    else if (strcmp(cmd, "GET_ROOMS") == 0) {
        printf(" DEBUG - Obteniendo salas...\n");
        
        Sala* salas = NULL;
        int num_salas = 0;
        
        if (sala_listar(&salas, &num_salas)) {
            printf(" DEBUG - Encontradas %d salas\n", num_salas);
            strcpy(respuesta, "OK:");
            
            for (int i = 0; i < num_salas; i++) {
                char temp[100];
                int asientos_libres = sala_contar_asientos_libres(salas[i].id);
                snprintf(temp, sizeof(temp), "%d|%d|%d;", 
                        salas[i].id, salas[i].numero_asientos, asientos_libres);
                strcat(respuesta, temp);
                printf("  - Sala %d: %d asientos\n", salas[i].id, salas[i].numero_asientos);
            }
            
            sala_liberar_lista(salas, num_salas);
            log_info("Enviadas %d salas al cliente", num_salas);
        } else {
            strcpy(respuesta, "ERROR:No se pudieron obtener salas");
            printf(" DEBUG - Error obteniendo salas\n");
            log_error("Error obteniendo lista de salas");
        }
    }
    else if (strcmp(cmd, "CREATE_ROOM") == 0) {
        printf(" DEBUG - Creando sala...\n");
        
        if (!auth_sesion_activa() || !auth_es_administrador()) {
            strcpy(respuesta, "ERROR:Acceso denegado - Requiere permisos de administrador");
            log_warning("Intento de crear sala sin permisos");
            free(comando_copia);
            return respuesta;
        }
        
        char* num_asientos_str = strtok(NULL, ":");
        
        if (!num_asientos_str) {
            strcpy(respuesta, "ERROR:Numero de asientos requerido");
            free(comando_copia);
            return respuesta;
        }
        
        Sala nueva_sala = {0};
        nueva_sala.numero_asientos = atoi(num_asientos_str);
        
        if (nueva_sala.numero_asientos <= 0 || nueva_sala.numero_asientos > 1000) {
            strcpy(respuesta, "ERROR:Numero de asientos invalido (1-1000)");
            free(comando_copia);
            return respuesta;
        }
        
        if (sala_crear(&nueva_sala)) {
            snprintf(respuesta, MAX_BUFFER, "OK:%d", nueva_sala.id);
            printf(" DEBUG - Sala creada con ID %d\n", nueva_sala.id);
            log_info("Sala creada con ID %d y %d asientos", nueva_sala.id, nueva_sala.numero_asientos);
        } else {
            strcpy(respuesta, "ERROR:No se pudo crear la sala");
            log_error("Error creando sala con %d asientos", nueva_sala.numero_asientos);
        }
    }
    else if (strcmp(cmd, "GET_SESSIONS") == 0) {
        printf(" DEBUG - Obteniendo sesiones...\n");
        
        Sesion* sesiones = NULL;
        int num_sesiones = 0;
        
        if (sesion_listar(&sesiones, &num_sesiones)) {
            printf(" DEBUG - Encontradas %d sesiones\n", num_sesiones);
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
            strcpy(respuesta, "ERROR:ID de pelicula requerido");
            free(comando_copia);
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
            log_info("Enviadas %d sesiones para pelicula %d", num_sesiones, movie_id);
        } else {
            strcpy(respuesta, "ERROR:No se encontraron sesiones para esta pelicula");
            log_info("No hay sesiones para pelicula %d", movie_id);
        }
    }
    else if (strcmp(cmd, "GET_USER_PURCHASES") == 0) {
        if (!auth_sesion_activa()) {
            strcpy(respuesta, "ERROR:Debe iniciar sesion");
            free(comando_copia);
            return respuesta;
        }
        
        char* user_id_str = strtok(NULL, ":");
        int user_id = user_id_str ? atoi(user_id_str) : auth_obtener_usuario_actual()->id;
        
        Usuario* current_user = auth_obtener_usuario_actual();
        if (current_user->tipo != USUARIO_ADMINISTRADOR && current_user->id != user_id) {
            strcpy(respuesta, "ERROR:No puede ver compras de otros usuarios");
            log_warning("Usuario %d intentó ver compras de usuario %d", current_user->id, user_id);
            free(comando_copia);
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
        // Comando para obtener información de una sala específica
    else if (strcmp(cmd, "GET_ROOM_INFO") == 0) {
        char* sala_id_str = strtok(NULL, ":");
        
        if (!sala_id_str) {
            strcpy(respuesta, "ERROR:ID de sala requerido");
            free(comando_copia);
            return respuesta;
        }
        
        int sala_id = atoi(sala_id_str);
        Sala sala;
        
        if (sala_obtener_por_id(sala_id, &sala)) {
            int asientos_libres = sala_contar_asientos_libres(sala_id);
            snprintf(respuesta, MAX_BUFFER, "OK:%d:%d:%d", 
                    sala.id, sala.numero_asientos, asientos_libres);
            printf(" DEBUG - Info sala %d enviada\n", sala_id);
            log_info("Informacion de sala %d enviada", sala_id);
        } else {
            strcpy(respuesta, "ERROR:Sala no encontrada");
            printf(" DEBUG - Sala %d no encontrada\n", sala_id);
            log_error("Sala %d no encontrada", sala_id);
        }
    }
    
    // Comando para obtener asientos ocupados de una sesión
    else if (strcmp(cmd, "GET_SESSION_SEATS") == 0) {
        char* sesion_id_str = strtok(NULL, ":");
        
        if (!sesion_id_str) {
            strcpy(respuesta, "ERROR:ID de sesion requerido");
            free(comando_copia);
            return respuesta;
        }
        
        int sesion_id = atoi(sesion_id_str);
        Sesion sesion;
        
        if (!sesion_obtener_por_id(sesion_id, &sesion)) {
            strcpy(respuesta, "ERROR:Sesion no encontrada");
            free(comando_copia);
            return respuesta;
        }
        
        // Obtener información de la sala
        Sala sala;
        if (!sala_obtener_por_id(sesion.sala_id, &sala)) {
            strcpy(respuesta, "ERROR:Sala no encontrada");
            free(comando_copia);
            return respuesta;
        }
        
        // Obtener billetes de esta sesión para saber qué asientos están ocupados
        Billete* billetes = NULL;
        int num_billetes = 0;
        
        if (billete_listar_por_sesion(sesion_id, &billetes, &num_billetes)) {
            // Crear lista de asientos ocupados
            char asientos_ocupados[512] = "";
            bool first = true;
            
            for (int i = 0; i < num_billetes; i++) {
                Asiento asiento;
                if (asiento_obtener_por_id(billetes[i].asiento_id, &asiento)) {
                    if (!first) {
                        strcat(asientos_ocupados, ",");
                    }
                    char num_str[10];
                    snprintf(num_str, sizeof(num_str), "%d", asiento.numero);
                    strcat(asientos_ocupados, num_str);
                    first = false;
                }
            }
            
            if (strlen(asientos_ocupados) == 0) {
                strcpy(asientos_ocupados, "NONE");
            }
            
            snprintf(respuesta, MAX_BUFFER, "OK:%d:%d:%s", 
                    sala.id, sala.numero_asientos, asientos_ocupados);
            
            billete_liberar_lista(billetes, num_billetes);
            printf(" DEBUG - Asientos de sesion %d enviados\n", sesion_id);
            log_info("Asientos de sesión %d enviados", sesion_id);
        } else {
            snprintf(respuesta, MAX_BUFFER, "OK:%d:%d:NONE", 
                    sala.id, sala.numero_asientos);
            printf(" DEBUG - Info asientos sesion %d (sin billetes)\n", sesion_id);
        }
    }
    
    // Comando para verificar si un asiento específico está disponible
    else if (strcmp(cmd, "CHECK_SEAT_AVAILABLE") == 0) {
        char* sesion_id_str = strtok(NULL, ":");
        char* numero_asiento_str = strtok(NULL, ":");
        
        if (!sesion_id_str || !numero_asiento_str) {
            strcpy(respuesta, "ERROR:Parametros incompletos");
            free(comando_copia);
            return respuesta;
        }
        
        int sesion_id = atoi(sesion_id_str);
        int numero_asiento = atoi(numero_asiento_str);
        
        // Obtener sesión
        Sesion sesion;
        if (!sesion_obtener_por_id(sesion_id, &sesion)) {
            strcpy(respuesta, "ERROR:Sesion no encontrada");
            free(comando_copia);
            return respuesta;
        }
        
        // Buscar el asiento por número en la sala
        Asiento* asientos = NULL;
        int num_asientos = 0;
        
        if (!asiento_listar_por_sala(sesion.sala_id, &asientos, &num_asientos)) {
            strcpy(respuesta, "ERROR:Error al obtener asientos");
            free(comando_copia);
            return respuesta;
        }
        
        int asiento_id = -1;
        for (int i = 0; i < num_asientos; i++) {
            if (asientos[i].numero == numero_asiento) {
                asiento_id = asientos[i].id;
                break;
            }
        }
        
        asiento_liberar_lista(asientos, num_asientos);
        
        if (asiento_id == -1) {
            strcpy(respuesta, "ERROR:Asiento no existe");
            free(comando_copia);
            return respuesta;
        }
        
        // Verificar disponibilidad
        if (billete_esta_disponible(sesion_id, asiento_id)) {
            strcpy(respuesta, "OK:AVAILABLE");
            printf(" DEBUG - Asiento %d disponible en sesion %d\n", numero_asiento, sesion_id);
        } else {
            strcpy(respuesta, "OK:OCCUPIED");
            printf(" DEBUG - Asiento %d ocupado en sesion %d\n", numero_asiento, sesion_id);
        }
    }
    
    // Comando para procesar compra de entradas
    else if (strcmp(cmd, "PURCHASE_TICKETS") == 0) {
        printf(" DEBUG - Procesando compra de entradas...\n");
        
        if (!auth_sesion_activa()) {
            strcpy(respuesta, "ERROR:Debe iniciar sesion");
            free(comando_copia);
            return respuesta;
        }
        
        char* usuario_id_str = strtok(NULL, ":");
        char* sesion_id_str = strtok(NULL, ":");
        char* num_entradas_str = strtok(NULL, ":");
        
        if (!usuario_id_str || !sesion_id_str || !num_entradas_str) {
            strcpy(respuesta, "ERROR:Parametros incompletos");
            free(comando_copia);
            return respuesta;
        }
        
        int usuario_id = atoi(usuario_id_str);
        int sesion_id = atoi(sesion_id_str);
        int num_entradas = atoi(num_entradas_str);
        
        if (num_entradas <= 0 || num_entradas > 10) {
            strcpy(respuesta, "ERROR:Numero de entradas invalido");
            free(comando_copia);
            return respuesta;
        }
        
        // Verificar que el usuario actual puede hacer esta compra
        Usuario* current_user = auth_obtener_usuario_actual();
        if (current_user->tipo != USUARIO_ADMINISTRADOR && current_user->id != usuario_id) {
            strcpy(respuesta, "ERROR:No puede comprar para otros usuarios");
            log_warning("Usuario %d intentó comprar para usuario %d", current_user->id, usuario_id);
            free(comando_copia);
            return respuesta;
        }
        
        // Verificar que la sesión existe
        Sesion sesion;
        if (!sesion_obtener_por_id(sesion_id, &sesion)) {
            strcpy(respuesta, "ERROR:Sesion no encontrada");
            free(comando_copia);
            return respuesta;
        }
        
        // Obtener los números de asientos seleccionados
        int asientos_seleccionados[10];
        for (int i = 0; i < num_entradas; i++) {
            char* asiento_str = strtok(NULL, ":");
            if (!asiento_str) {
                strcpy(respuesta, "ERROR:Faltan asientos seleccionados");
                free(comando_copia);
                return respuesta;
            }
            asientos_seleccionados[i] = atoi(asiento_str);
        }
        
        // Iniciar transacción
        if (!db_begin_transaction()) {
            strcpy(respuesta, "ERROR:Error al iniciar transaccion");
            log_error("Error al iniciar transacción para compra");
            free(comando_copia);
            return respuesta;
        }
        
        // Crear billetes para cada asiento
        Billete billetes[10];
        bool error = false;
        double precio_unitario = 8.50;
        
        for (int i = 0; i < num_entradas && !error; i++) {
            // Buscar el ID del asiento por su número
            Asiento* asientos = NULL;
            int num_asientos = 0;
            
            if (!asiento_listar_por_sala(sesion.sala_id, &asientos, &num_asientos)) {
                log_error("Error al obtener asientos de la sala");
                error = true;
                break;
            }
            
            int asiento_id = -1;
            for (int j = 0; j < num_asientos; j++) {
                if (asientos[j].numero == asientos_seleccionados[i]) {
                    asiento_id = asientos[j].id;
                    break;
                }
            }
            
            asiento_liberar_lista(asientos, num_asientos);
            
            if (asiento_id == -1) {
                log_error("Asiento %d no encontrado", asientos_seleccionados[i]);
                error = true;
                break;
            }
            
            // Verificar disponibilidad
            if (!billete_esta_disponible(sesion_id, asiento_id)) {
                log_error("Asiento %d no disponible", asientos_seleccionados[i]);
                error = true;
                break;
            }
            
            // Crear el billete
            memset(&billetes[i], 0, sizeof(Billete));
            billetes[i].sesion_id = sesion_id;
            billetes[i].asiento_id = asiento_id;
            billetes[i].precio = precio_unitario;
            
            if (!billete_crear(&billetes[i])) {
                log_error("Error al crear billete para asiento %d", asientos_seleccionados[i]);
                error = true;
                break;
            }
            
            printf(" DEBUG - Billete creado: ID %d, Asiento %d\n", billetes[i].id, asientos_seleccionados[i]);
        }
        
        if (error) {
            db_rollback_transaction();
            strcpy(respuesta, "ERROR:Error al crear billetes");
            free(comando_copia);
            return respuesta;
        }
        
        // Crear la venta
        Venta venta = {0};
        venta.usuario_id = usuario_id;
        venta.descuento = 0.0;
        venta.precio_total = precio_unitario * num_entradas;
        
        // Establecer fecha actual
        time_t now = time(NULL);
        struct tm* tm_info = localtime(&now);
        strftime(venta.fecha, sizeof(venta.fecha), "%Y-%m-%d %H:%M:%S", tm_info);
        
        // Insertar la venta
        char sql[512];
        snprintf(sql, sizeof(sql),
                "INSERT INTO Venta (Usuario_ID, Fecha, Descuento, PrecioTotal) "
                "VALUES (%d, '%s', %.2f, %.2f);",
                venta.usuario_id, venta.fecha, venta.descuento, venta.precio_total);
        
        if (!db_execute(sql)) {
            log_error("Error al crear la venta");
            db_rollback_transaction();
            strcpy(respuesta, "ERROR:Error al crear venta");
            free(comando_copia);
            return respuesta;
        }
        
        venta.id = db_last_insert_id();
        
        // Asociar billetes a la venta
        for (int i = 0; i < num_entradas; i++) {
            snprintf(sql, sizeof(sql),
                    "INSERT INTO Venta_Billetes (Venta_ID, Billete_ID) "
                    "VALUES (%d, %d);",
                    venta.id, billetes[i].id);
            
            if (!db_execute(sql)) {
                log_error("Error al asociar billete %d a venta %d", billetes[i].id, venta.id);
                db_rollback_transaction();
                strcpy(respuesta, "ERROR:Error al asociar billetes");
                free(comando_copia);
                return respuesta;
            }
        }
        
        // Confirmar transacción
        if (!db_commit_transaction()) {
            log_error("Error al confirmar transacción de compra");
            db_rollback_transaction();
            strcpy(respuesta, "ERROR:Error al finalizar compra");
            free(comando_copia);
            return respuesta;
        }
        
        snprintf(respuesta, MAX_BUFFER, "OK:%d", venta.id);
        printf(" DEBUG - Compra completada: Venta ID %d, %d entradas, Total %.2f\n", 
               venta.id, num_entradas, venta.precio_total);
        log_info("Compra completada: Usuario %d, Venta %d, %d entradas", 
                usuario_id, venta.id, num_entradas);
    }
    
    // Comando para obtener detalles de una compra específica
    else if (strcmp(cmd, "GET_PURCHASE_DETAILS") == 0) {
        char* venta_id_str = strtok(NULL, ":");
        
        if (!venta_id_str) {
            strcpy(respuesta, "ERROR:ID de venta requerido");
            free(comando_copia);
            return respuesta;
        }
        
        int venta_id = atoi(venta_id_str);
        
        // Verificar autenticación
        if (!auth_sesion_activa()) {
            strcpy(respuesta, "ERROR:Debe iniciar sesion");
            free(comando_copia);
            return respuesta;
        }
        
        // Obtener la venta
        Venta venta;
        if (!venta_obtener_por_id(venta_id, &venta)) {
            strcpy(respuesta, "ERROR:Venta no encontrada");
            free(comando_copia);
            return respuesta;
        }
        
        // Verificar permisos
        Usuario* current_user = auth_obtener_usuario_actual();
        if (current_user->tipo != USUARIO_ADMINISTRADOR && current_user->id != venta.usuario_id) {
            strcpy(respuesta, "ERROR:No puede ver compras de otros usuarios");
            log_warning("Usuario %d intentó ver compra de usuario %d", current_user->id, venta.usuario_id);
            free(comando_copia);
            return respuesta;
        }
        
        // Obtener billetes de esta venta
        Billete* billetes = NULL;
        int num_billetes = 0;
        
        if (!venta_obtener_billetes(venta_id, &billetes, &num_billetes)) {
            snprintf(respuesta, MAX_BUFFER, "OK:%s:%.2f:0:", venta.fecha, venta.precio_total);
            free(comando_copia);
            return respuesta;
        }
        
        // Construir información detallada de billetes
        char billetes_info[2048] = "";
        bool first = true;
        
        for (int i = 0; i < num_billetes; i++) {
            Sesion sesion;
            Pelicula pelicula;
            Asiento asiento;
            
            if (sesion_obtener_por_id(billetes[i].sesion_id, &sesion) &&
                pelicula_obtener_por_id(sesion.pelicula_id, &pelicula) &&
                asiento_obtener_por_id(billetes[i].asiento_id, &asiento)) {
                
                if (!first) {
                    strcat(billetes_info, ";");
                }
                
                char billete_str[300];
                char fecha_sesion[11];
                strncpy(fecha_sesion, sesion.hora_inicio, 10);
                fecha_sesion[10] = '\0';
                
                snprintf(billete_str, sizeof(billete_str), "%d|%s|%s|%d|%d",
                        billetes[i].id, pelicula.titulo, fecha_sesion, 
                        sesion.sala_id, asiento.numero);
                
                strcat(billetes_info, billete_str);
                first = false;
            }
        }
        
        snprintf(respuesta, MAX_BUFFER, "OK:%s:%.2f:%d:%s", 
                venta.fecha, venta.precio_total, num_billetes, billetes_info);
        
        billete_liberar_lista(billetes, num_billetes);
        printf(" DEBUG - Detalles de venta %d enviados\n", venta_id);
        log_info("Detalles de venta %d enviados a usuario %d", venta_id, current_user->id);
    }
    else if (strcmp(cmd, "QUIT") == 0) {
        strcpy(respuesta, "BYE");
        log_info("Cliente solicita desconexión");
    }
    else if (strcmp(cmd, "DELETE_MOVIE") == 0) {
        char* id_str = strtok(NULL, ":");
        if (!id_str) {
            strcpy(respuesta, "ERROR:ID de película requerido");
            free(comando_copia);
            return respuesta;
        }
        int id = atoi(id_str);
        char sql[128];
        snprintf(sql, sizeof(sql), "DELETE FROM Pelicula WHERE ID = %d;", id);

        if (db_execute(sql)) {
            snprintf(respuesta, MAX_BUFFER, "OK:Pelicula eliminada");
        } else {
            snprintf(respuesta, MAX_BUFFER, "ERROR:No se pudo eliminar la pelicula");
        }
    }
    else if (strcmp(cmd, "DELETE_ROOM") == 0) {
        char* id_str = strtok(NULL, ":");
        if (!id_str) {
            strcpy(respuesta, "ERROR:ID de sala requerido");
            free(comando_copia);
            return respuesta;
        }
        int id = atoi(id_str);
        char sql[128];
        snprintf(sql, sizeof(sql), "DELETE FROM Sala WHERE ID = %d;", id);

        if (db_execute(sql)) {
            snprintf(respuesta, MAX_BUFFER, "OK:Sala eliminada");
        } else {
            snprintf(respuesta, MAX_BUFFER, "ERROR:No se pudo eliminar la sala");
        }
    }
    else if (strcmp(cmd, "DELETE_SESSION") == 0) {
        char* id_str = strtok(NULL, ":");
        if (!id_str) {
            strcpy(respuesta, "ERROR:ID de sesión requerido");
            free(comando_copia);
            return respuesta;
        }
        int id = atoi(id_str);
        char sql[128];
        snprintf(sql, sizeof(sql), "DELETE FROM Sesion WHERE ID = %d;", id);

        if (db_execute(sql)) {
            snprintf(respuesta, MAX_BUFFER, "OK:Sesion eliminada");
        } else {
            snprintf(respuesta, MAX_BUFFER, "ERROR:No se pudo eliminar la sesion");
        }
    }
    else if (strcmp(cmd, "CREATE_SESSION") == 0) {
        char* pelicula_id_str = strtok(NULL, ":");
        char* sala_id_str = strtok(NULL, ":");
        char* inicio = strtok(NULL, ":");
        char* fin = strtok(NULL, ":");

        if (!pelicula_id_str || !sala_id_str || !inicio || !fin) {
            snprintf(respuesta, MAX_BUFFER, "ERROR:Formato invalido");
        } else {
            int pelicula_id = atoi(pelicula_id_str);
            int sala_id = atoi(sala_id_str);

            char sql[256];
            snprintf(sql, sizeof(sql),
                     "INSERT INTO Sesion (Pelicula_ID, Sala_ID, Hora_Inicio, Hora_Fin) "
                     "VALUES (%d, %d, '%s', '%s');",
                     pelicula_id, sala_id, inicio, fin);

            if (db_execute(sql)) {
                snprintf(respuesta, MAX_BUFFER, "OK:Sesion creada");
            } else {
                snprintf(respuesta, MAX_BUFFER, "ERROR:No se pudo crear la sesion");
            }
        }
    }
    else if (strcmp(cmd, "GET_USERS") == 0) {
        if (!auth_sesion_activa() || !auth_es_administrador()) {
            strcpy(respuesta, "ERROR:Permiso denegado");
        } else {
            Usuario* lista = NULL;
            int cantidad = 0;
            if (usuario_listar(&lista, &cantidad)) {
                strcpy(respuesta, "OK:");
                for (int i = 0; i < cantidad; i++) {
                    char linea[256];
                    snprintf(linea, sizeof(linea), "%d|%s|%s|%s;", 
                             lista[i].id, lista[i].nombre, 
                             lista[i].tipo == USUARIO_ADMINISTRADOR ? "admin" : "cliente",
                             lista[i].correo);
                    strcat(respuesta, linea);
                }
                usuario_liberar_lista(lista, cantidad);
            } else {
                strcpy(respuesta, "ERROR:No se pudo obtener usuarios");
            }
        }
    }
    /*else if (strcmp(cmd, "CREATE_USER") == 0) {
        if (!auth_sesion_activa() || !auth_es_administrador()) {
            strcpy(respuesta, "ERROR:Permiso denegado");
        } else {
            char* nombre = strtok(NULL, ":");
            char* correo = strtok(NULL, ":");
            char* password = strtok(NULL, ":");
            char* tipo = strtok(NULL, ":");

            if (!nombre || !correo || !password || !tipo) {
                strcpy(respuesta, "ERROR:Datos incompletos");
            } else {
                Usuario nuevo = {0};
                strncpy(nuevo.nombre, nombre, sizeof(nuevo.nombre)-1);
                strncpy(nuevo.correo, correo, sizeof(nuevo.correo)-1);
                strncpy(nuevo.contrasena, password, sizeof(nuevo.contrasena)-1);
                nuevo.tipo = (strcmp(tipo, "admin") == 0) ? USUARIO_ADMINISTRADOR : USUARIO_CLIENTE;

                if (usuario_crear(&nuevo)) {
                    snprintf(respuesta, MAX_BUFFER, "OK:%d", nuevo.id);
                } else {
                    strcpy(respuesta, "ERROR:No se pudo crear el usuario");
                }
            }
        }
    }*/
    else if (strcmp(cmd, "DELETE_USER") == 0) {
        if (!auth_sesion_activa() || !auth_es_administrador()) {
            strcpy(respuesta, "ERROR:Permiso denegado");
        } else {
            char* id_str = strtok(NULL, ":");
            if (!id_str) {
                strcpy(respuesta, "ERROR:Falta ID");
            } else {
                int id = atoi(id_str);
                if (usuario_eliminar(id)) {
                    strcpy(respuesta, "OK:Usuario eliminado");
                } else {
                    strcpy(respuesta, "ERROR:No se pudo eliminar el usuario");
                }
            }
        }
    }
    else if (strcmp(cmd, "CREATE_USER") == 0) {
    char* nombre = strtok(NULL, ":");
    char* correo = strtok(NULL, ":");
    char* password = strtok(NULL, ":");
    char* tipo = strtok(NULL, ":");
    char* telefono = strtok(NULL, ":");

    if (!nombre || !correo || !password || !tipo) {
        strcpy(respuesta, "ERROR:Datos incompletos");
    } else {
        Usuario nuevo = {0};
        strncpy(nuevo.nombre, nombre, sizeof(nuevo.nombre) - 1);
        strncpy(nuevo.correo, correo, sizeof(nuevo.correo) - 1);
        strncpy(nuevo.contrasena, password, sizeof(nuevo.contrasena) - 1);
        nuevo.tipo = (strcmp(tipo, "admin") == 0) ? USUARIO_ADMINISTRADOR : USUARIO_CLIENTE;
        if (telefono)
            strncpy(nuevo.telefono, telefono, sizeof(nuevo.telefono) - 1);

        if (usuario_crear(&nuevo)) {
            snprintf(respuesta, MAX_BUFFER, "OK:%d", nuevo.id);
        } else {
            strcpy(respuesta, "ERROR:No se pudo crear el usuario");
        }
    }
}




    else {
        snprintf(respuesta, MAX_BUFFER, "ERROR:Comando no reconocido: %s", cmd);
        printf(" DEBUG - Comando no reconocido: %s\n", cmd);
        log_warning("Comando no reconocido: %s", cmd);
    }
    
    free(comando_copia);
    printf(" DEBUG - Respuesta: %.100s%s\n", respuesta, strlen(respuesta) > 100 ? "..." : "");
    return respuesta;
}

int main() {
    #ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2,2), &wsaData) != 0) {
        fprintf(stderr, "Error inicializando Winsock\n");
        return -1;
    }
    #endif
    
    printf("=== SERVIDOR DE GESTION DE CINE ===\n");
    
    // Inicializar sistema
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
    printf(" Base de datos inicializada correctamente\n");
    
    printf("Inicializando datos de prueba...\n");
    if (test_data_init()) {
        printf(" Datos de prueba cargados exitosamente\n");
        log_info("Datos de prueba cargados exitosamente");
    } else {
        printf(" Los datos de prueba no se cargaron (puede que ya existan)\n");
        log_warning("Los datos de prueba no se cargaron");
    }
    
    // Verificar datos
    printf("Verificando datos en la base de datos...\n");
    
    Usuario* usuarios = NULL;
    int num_usuarios = 0;
    if (usuario_listar(&usuarios, &num_usuarios)) {
        printf(" Usuarios en BD: %d\n", num_usuarios);
        for (int i = 0; i < num_usuarios; i++) {
            printf("   - %s (%s)\n", usuarios[i].correo, 
                   usuarios[i].tipo == USUARIO_ADMINISTRADOR ? "Admin" : "Cliente");
        }
        usuario_liberar_lista(usuarios, num_usuarios);
    }
    
    Pelicula* peliculas = NULL;
    int num_peliculas = 0;
    if (pelicula_listar(&peliculas, &num_peliculas)) {
        printf(" Películas en BD: %d\n", num_peliculas);
        for (int i = 0; i < num_peliculas && i < 3; i++) {
            printf("   - %s (%d min)\n", peliculas[i].titulo, peliculas[i].duracion);
        }
        if (num_peliculas > 3) printf("   ... y %d más\n", num_peliculas - 3);
        pelicula_liberar_lista(peliculas, num_peliculas);
    }
    
    Sala* salas = NULL;
    int num_salas = 0;
    if (sala_listar(&salas, &num_salas)) {
        printf(" Salas en BD: %d\n", num_salas);
        for (int i = 0; i < num_salas; i++) {
            printf("   - Sala %d: %d asientos\n", salas[i].id, salas[i].numero_asientos);
        }
        sala_liberar_lista(salas, num_salas);
    }

    auth_init();
    log_info("Sistema inicializado correctamente");
    
    // Iniciar servidor de red
    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    char buffer[MAX_BUFFER] = {0};

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Fallo en socket");
        log_critical("Error creando socket");
        return -1;
    }

    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt))) {
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

    printf("\n Servidor escuchando en puerto %d\n", PORT);
    printf("Esperando conexiones de clientes...\n\n");
    log_info("Servidor escuchando en puerto %d", PORT);

    while (1) {
        printf(" Esperando cliente...\n");
        
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, 
                                (socklen_t*)&addrlen)) < 0) {
            perror("Fallo en accept");
            log_error("Error aceptando conexión");
            continue;
        }
        
        printf(" Cliente conectado desde %s\n", inet_ntoa(address.sin_addr));
        log_info("Cliente conectado desde %s", inet_ntoa(address.sin_addr));

        // Leer comando del cliente
        memset(buffer, 0, MAX_BUFFER);
        int bytes_received = recv(new_socket, buffer, MAX_BUFFER - 1, 0);
        
        if (bytes_received > 0) {
            buffer[bytes_received] = '\0';
            printf(" Comando recibido: %s\n", buffer);

            // Procesar comando
            char* respuesta = procesar_comando(buffer);
            
            // Enviar respuesta
            send(new_socket, respuesta, strlen(respuesta), 0);
            printf(" Respuesta enviada: %.100s%s\n", 
                   respuesta, strlen(respuesta) > 100 ? "..." : "");

            free(respuesta);
        } else {
            printf(" Error recibiendo datos del cliente\n");
            log_error("Error recibiendo datos del cliente");
        }

        close(new_socket);
        printf(" Cliente desconectado\n\n");

        // Si es QUIT, terminar servidor
        if (strcmp(buffer, "QUIT") == 0) {
            printf(" Comando QUIT recibido, cerrando servidor...\n");
            log_info("Servidor cerrado por comando QUIT");
            break;
        }
    }

    close(server_fd);
    
    // Limpiar sistema
    log_info("===== CERRANDO SERVIDOR =====");
    db_close();
    log_close();
    memory_cleanup();
    
    printf("Servidor cerrado correctamente\n");
    #ifdef _WIN32
    WSACleanup();
    #endif
    
    return 0;
}