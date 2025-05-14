#include "test_data.h"
#include "database.h"
#include "models/usuario.h"
#include "models/pelicula.h"
#include "models/sala.h"
#include "models/asiento.h"
#include "models/sesion.h"
#include "models/billete.h"
#include "models/venta.h"
#include "utils/logger.h"
#include <stdio.h>
#include <string.h>
#include <time.h>

// Función para crear usuarios de prueba
static bool crear_usuarios_prueba() {
    log_info("Creando usuarios de prueba...");
    
    // Administrador (ya debería existir por la configuración)
    Usuario admin = {0};
    strcpy(admin.nombre, "Administrador");
    strcpy(admin.correo, "admin@cinegestion.com");
    strcpy(admin.contrasena, "admin123");
    strcpy(admin.telefono, "123456789");
    admin.tipo = USUARIO_ADMINISTRADOR;
    
    if (!usuario_crear(&admin)) {
        log_warning("El administrador ya existe o no se pudo crear");
    }
    
    // Cliente 1
    Usuario cliente1 = {0};
    strcpy(cliente1.nombre, "Juan Pérez");
    strcpy(cliente1.correo, "juan@example.com");
    strcpy(cliente1.contrasena, "password1");
    strcpy(cliente1.telefono, "611223344");
    cliente1.tipo = USUARIO_CLIENTE;
    
    if (!usuario_crear(&cliente1)) {
        log_error("No se pudo crear cliente1");
        return false;
    }
    
    // Cliente 2
    Usuario cliente2 = {0};
    strcpy(cliente2.nombre, "María López");
    strcpy(cliente2.correo, "maria@example.com");
    strcpy(cliente2.contrasena, "password2");
    strcpy(cliente2.telefono, "622334455");
    cliente2.tipo = USUARIO_CLIENTE;
    
    if (!usuario_crear(&cliente2)) {
        log_error("No se pudo crear cliente2");
        return false;
    }
    
    // Cliente 3
    Usuario cliente3 = {0};
    strcpy(cliente3.nombre, "Carlos Ruiz");
    strcpy(cliente3.correo, "carlos@example.com");
    strcpy(cliente3.contrasena, "password3");
    strcpy(cliente3.telefono, "633445566");
    cliente3.tipo = USUARIO_CLIENTE;
    
    if (!usuario_crear(&cliente3)) {
        log_error("No se pudo crear cliente3");
        return false;
    }
    
    log_info("Usuarios de prueba creados correctamente");
    return true;
}

// Función para crear películas de prueba
static bool crear_peliculas_prueba() {
    log_info("Creando películas de prueba...");
    
    // Película 1
    Pelicula pelicula1 = {0};
    strcpy(pelicula1.titulo, "El Padrino");
    pelicula1.duracion = 175;
    strcpy(pelicula1.genero, "Drama, Crimen");
    
    if (!pelicula_crear(&pelicula1)) {
        log_error("No se pudo crear pelicula1");
        return false;
    }
    
    // Película 2
    Pelicula pelicula2 = {0};
    strcpy(pelicula2.titulo, "El Señor de los Anillos");
    pelicula2.duracion = 178;
    strcpy(pelicula2.genero, "Fantasía, Aventura");
    
    if (!pelicula_crear(&pelicula2)) {
        log_error("No se pudo crear pelicula2");
        return false;
    }
    
    // Película 3
    Pelicula pelicula3 = {0};
    strcpy(pelicula3.titulo, "Matrix");
    pelicula3.duracion = 136;
    strcpy(pelicula3.genero, "Ciencia Ficción, Acción");
    
    if (!pelicula_crear(&pelicula3)) {
        log_error("No se pudo crear pelicula3");
        return false;
    }
    
    // Película 4
    Pelicula pelicula4 = {0};
    strcpy(pelicula4.titulo, "Titanic");
    pelicula4.duracion = 195;
    strcpy(pelicula4.genero, "Romance, Drama");
    
    if (!pelicula_crear(&pelicula4)) {
        log_error("No se pudo crear pelicula4");
        return false;
    }
    
    // Película 5
    Pelicula pelicula5 = {0};
    strcpy(pelicula5.titulo, "Star Wars: Episodio IV");
    pelicula5.duracion = 121;
    strcpy(pelicula5.genero, "Ciencia Ficción, Aventura");
    
    if (!pelicula_crear(&pelicula5)) {
        log_error("No se pudo crear pelicula5");
        return false;
    }
    
    log_info("Películas de prueba creadas correctamente");
    return true;
}

// Función para crear salas de prueba
static bool crear_salas_prueba() {
    log_info("Creando salas de prueba...");
    
    // Sala 1
    Sala sala1 = {0};
    sala1.numero_asientos = 50;
    
    if (!sala_crear(&sala1)) {
        log_error("No se pudo crear sala1");
        return false;
    }
    
    // Sala 2
    Sala sala2 = {0};
    sala2.numero_asientos = 80;
    
    if (!sala_crear(&sala2)) {
        log_error("No se pudo crear sala2");
        return false;
    }
    
    // Sala 3
    Sala sala3 = {0};
    sala3.numero_asientos = 120;
    
    if (!sala_crear(&sala3)) {
        log_error("No se pudo crear sala3");
        return false;
    }
    
    log_info("Salas de prueba creadas correctamente");
    return true;
}

// Función para crear sesiones de prueba
static bool crear_sesiones_prueba() {
    log_info("Creando sesiones de prueba...");
    
    // Sesión 1: El Padrino en Sala 1 (hoy a las 16:00)
    Sesion sesion1 = {0};
    sesion1.pelicula_id = 1;
    sesion1.sala_id = 1;
    
    time_t ahora = time(NULL);
    struct tm* tm_hoy = localtime(&ahora);
    
    // Configurar para hoy a las 16:00
    tm_hoy->tm_hour = 16;
    tm_hoy->tm_min = 0;
    tm_hoy->tm_sec = 0;
    
    sesion_convertir_time_a_str(tm_hoy, sesion1.hora_inicio, sizeof(sesion1.hora_inicio));
    
    // Calcular hora de fin (duración película + 15 min)
    time_t tiempo_inicio = mktime(tm_hoy);
    time_t tiempo_fin = tiempo_inicio + (175 + 15) * 60; // El Padrino: 175 min + 15 min limpieza
    struct tm* tm_fin = localtime(&tiempo_fin);
    
    sesion_convertir_time_a_str(tm_fin, sesion1.hora_fin, sizeof(sesion1.hora_fin));
    
    if (!sesion_crear(&sesion1)) {
        log_error("No se pudo crear sesion1");
        return false;
    }
    
    // Sesión 2: El Señor de los Anillos en Sala 2 (hoy a las 18:00)
    Sesion sesion2 = {0};
    sesion2.pelicula_id = 2;
    sesion2.sala_id = 2;
    
    // Configurar para hoy a las 18:00
    tm_hoy->tm_hour = 18;
    tm_hoy->tm_min = 0;
    tm_hoy->tm_sec = 0;
    
    sesion_convertir_time_a_str(tm_hoy, sesion2.hora_inicio, sizeof(sesion2.hora_inicio));
    
    // Calcular hora de fin
    tiempo_inicio = mktime(tm_hoy);
    tiempo_fin = tiempo_inicio + (178 + 15) * 60; // El Señor de los Anillos: 178 min + 15 min limpieza
    tm_fin = localtime(&tiempo_fin);
    
    sesion_convertir_time_a_str(tm_fin, sesion2.hora_fin, sizeof(sesion2.hora_fin));
    
    if (!sesion_crear(&sesion2)) {
        log_error("No se pudo crear sesion2");
        return false;
    }
    
    // Sesión 3: Matrix en Sala 3 (hoy a las 20:00)
    Sesion sesion3 = {0};
    sesion3.pelicula_id = 3;
    sesion3.sala_id = 3;
    
    // Configurar para hoy a las 20:00
    tm_hoy->tm_hour = 20;
    tm_hoy->tm_min = 0;
    tm_hoy->tm_sec = 0;
    
    sesion_convertir_time_a_str(tm_hoy, sesion3.hora_inicio, sizeof(sesion3.hora_inicio));
    
    // Calcular hora de fin
    tiempo_inicio = mktime(tm_hoy);
    tiempo_fin = tiempo_inicio + (136 + 15) * 60; // Matrix: 136 min + 15 min limpieza
    tm_fin = localtime(&tiempo_fin);
    
    sesion_convertir_time_a_str(tm_fin, sesion3.hora_fin, sizeof(sesion3.hora_fin));
    
    if (!sesion_crear(&sesion3)) {
        log_error("No se pudo crear sesion3");
        return false;
    }
    
    // Sesión 4: Titanic en Sala 1 (mañana a las 17:00)
    Sesion sesion4 = {0};
    sesion4.pelicula_id = 4;
    sesion4.sala_id = 1;
    
    // Configurar para mañana a las 17:00
    tm_hoy->tm_mday += 1; // Día siguiente
    tm_hoy->tm_hour = 17;
    tm_hoy->tm_min = 0;
    tm_hoy->tm_sec = 0;
    
    sesion_convertir_time_a_str(tm_hoy, sesion4.hora_inicio, sizeof(sesion4.hora_inicio));
    
    // Calcular hora de fin
    tiempo_inicio = mktime(tm_hoy);
    tiempo_fin = tiempo_inicio + (195 + 15) * 60; // Titanic: 195 min + 15 min limpieza
    tm_fin = localtime(&tiempo_fin);
    
    sesion_convertir_time_a_str(tm_fin, sesion4.hora_fin, sizeof(sesion4.hora_fin));
    
    if (!sesion_crear(&sesion4)) {
        log_error("No se pudo crear sesion4");
        return false;
    }
    
    // Sesión 5: Star Wars en Sala 2 (mañana a las 19:00)
    Sesion sesion5 = {0};
    sesion5.pelicula_id = 5;
    sesion5.sala_id = 2;
    
    // Configurar para mañana a las 19:00
    tm_hoy->tm_hour = 19;
    tm_hoy->tm_min = 0;
    tm_hoy->tm_sec = 0;
    
    sesion_convertir_time_a_str(tm_hoy, sesion5.hora_inicio, sizeof(sesion5.hora_inicio));
    
    // Calcular hora de fin
    tiempo_inicio = mktime(tm_hoy);
    tiempo_fin = tiempo_inicio + (121 + 15) * 60; // Star Wars: 121 min + 15 min limpieza
    tm_fin = localtime(&tiempo_fin);
    
    sesion_convertir_time_a_str(tm_fin, sesion5.hora_fin, sizeof(sesion5.hora_fin));
    
    if (!sesion_crear(&sesion5)) {
        log_error("No se pudo crear sesion5");
        return false;
    }
    
    log_info("Sesiones de prueba creadas correctamente");
    return true;
}

// Función para crear compras de prueba
static bool crear_compras_prueba() {
    log_info("Creando compras de prueba...");
    
    // Compra 1: Juan compra 2 entradas para El Padrino
    // Primero, crear los billetes
    Billete billetes1[2] = {0};
    
    billetes1[0].sesion_id = 1; // Sesión de El Padrino
    billetes1[0].asiento_id = 1; // Asiento 1
    billetes1[0].precio = 8.50;
    
    billetes1[1].sesion_id = 1; // Sesión de El Padrino
    billetes1[1].asiento_id = 2; // Asiento 2
    billetes1[1].precio = 8.50;
    
    // Crear la venta
    Venta venta1 = {0};
    venta1.usuario_id = 2; // Juan (asumiendo que es el ID 2)
    time_t ahora = time(NULL);
    struct tm* tm_ahora = localtime(&ahora);
    strftime(venta1.fecha, sizeof(venta1.fecha), "%Y-%m-%d %H:%M:%S", tm_ahora);
    venta1.descuento = 0; // Sin descuento
    
    if (!venta_crear(&venta1, billetes1, 2)) {
        log_error("No se pudo crear venta1");
        return false;
    }
    
    // Compra 2: María compra 3 entradas para El Señor de los Anillos
    Billete billetes2[3] = {0};
    
    billetes2[0].sesion_id = 2; // Sesión de El Señor de los Anillos
    billetes2[0].asiento_id = 51; // Asiento 1 de la Sala 2
    billetes2[0].precio = 8.50;
    
    billetes2[1].sesion_id = 2; // Sesión de El Señor de los Anillos
    billetes2[1].asiento_id = 52; // Asiento 2 de la Sala 2
    billetes2[1].precio = 8.50;
    
    billetes2[2].sesion_id = 2; // Sesión de El Señor de los Anillos
    billetes2[2].asiento_id = 53; // Asiento 3 de la Sala 2
    billetes2[2].precio = 8.50;
    
    // Crear la venta
    Venta venta2 = {0};
    venta2.usuario_id = 3; // María (asumiendo que es el ID 3)
    strftime(venta2.fecha, sizeof(venta2.fecha), "%Y-%m-%d %H:%M:%S", tm_ahora);
    venta2.descuento = 10; // 10% de descuento
    
    if (!venta_crear(&venta2, billetes2, 3)) {
        log_error("No se pudo crear venta2");
        return false;
    }
    
    log_info("Compras de prueba creadas correctamente");
    return true;
}

// Función principal para inicializar todos los datos de prueba
bool test_data_init() {
    log_info("Inicializando datos de prueba...");
    
    if (!crear_usuarios_prueba()) {
        return false;
    }
    
    if (!crear_peliculas_prueba()) {
        return false;
    }
    
    if (!crear_salas_prueba()) {
        return false;
    }
    
    if (!crear_sesiones_prueba()) {
        return false;
    }
    
    if (!crear_compras_prueba()) {
        return false;
    }
    
    log_info("Datos de prueba inicializados correctamente");
    return true;
}