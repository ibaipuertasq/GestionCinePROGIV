#include "test_data.h"
#include "models/usuario.h"
#include "models/pelicula.h"
#include "models/sala.h"
#include "models/sesion.h"
#include "models/venta.h"
#include "models/billete.h"
#include "utils/logger.h"
#include "utils/database.h"
#include <stdio.h>
#include <string.h>

// Función principal para inicializar datos de prueba
bool test_data_init() {
    printf("=== INICIANDO CARGA DE DATOS DE PRUEBA ===\n");
    log_info("=== INICIANDO CARGA DE DATOS DE PRUEBA ===");
    
    // Verificar si ya hay datos
    if (test_data_verificar_datos_existentes()) {
        printf("ℹ️  Ya existen datos en la base de datos, omitiendo carga\n");
        log_info("Ya existen datos en la base de datos, omitiendo carga de datos de prueba");
        return true;
    }
    
    printf("📝 No se encontraron datos, procediendo a crear datos de prueba...\n");
    
    // Crear datos de prueba en orden
    if (!test_data_crear_usuarios()) {
        printf("❌ Error creando usuarios de prueba\n");
        log_error("Error creando usuarios de prueba");
        return false;
    }
    
    if (!test_data_crear_peliculas()) {
        printf("❌ Error creando películas de prueba\n");
        log_error("Error creando películas de prueba");
        return false;
    }
    
    if (!test_data_crear_salas()) {
        printf("❌ Error creando salas de prueba\n");
        log_error("Error creando salas de prueba");
        return false;
    }
    
    if (!test_data_crear_sesiones()) {
        printf("❌ Error creando sesiones de prueba\n");
        log_error("Error creando sesiones de prueba");
        return false;
    }
    
    if (!test_data_crear_ventas_ejemplo()) {
        printf("⚠️  No se pudieron crear ventas de ejemplo (normal si no hay billetes)\n");
        log_info("No se pudieron crear ventas de ejemplo (esto es normal si no hay billetes)");
    }
    
    printf("=== ✅ DATOS DE PRUEBA CARGADOS EXITOSAMENTE ===\n\n");
    log_info("=== DATOS DE PRUEBA CARGADOS EXITOSAMENTE ===");
    return true;
}

// Crear usuarios de prueba
bool test_data_crear_usuarios() {
    printf("👥 Creando usuarios de prueba...\n");
    log_info("Creando usuarios de prueba...");
    
    // Usuario cliente 1 - ¡CORREO CORREGIDO!
    Usuario cliente1 = {0};
    strcpy(cliente1.nombre, "Juan Pérez");
    strcpy(cliente1.correo, "juan@email.com");  // ✅ CORREGIDO
    strcpy(cliente1.contrasena, "123456");
    strcpy(cliente1.telefono, "666123456");
    cliente1.tipo = USUARIO_CLIENTE;
    
    if (!usuario_crear(&cliente1)) {
        printf("❌ Error creando usuario Juan\n");
        log_error("Error creando cliente1");
        return false;
    }
    printf("✅ Usuario Juan creado (ID: %d) - Correo: %s\n", cliente1.id, cliente1.correo);
    
    // Usuario cliente 2 - ¡CORREO CORREGIDO!
    Usuario cliente2 = {0};
    strcpy(cliente2.nombre, "María García");
    strcpy(cliente2.correo, "maria@email.com");  // ✅ CORREGIDO
    strcpy(cliente2.contrasena, "123456");
    strcpy(cliente2.telefono, "666789012");
    cliente2.tipo = USUARIO_CLIENTE;
    
    if (!usuario_crear(&cliente2)) {
        printf("❌ Error creando usuario María\n");
        log_error("Error creando cliente2");
        return false;
    }
    printf("✅ Usuario María creado (ID: %d) - Correo: %s\n", cliente2.id, cliente2.correo);
    
    // Usuario cliente 3 - ¡CORREO CORREGIDO!
    Usuario cliente3 = {0};
    strcpy(cliente3.nombre, "Pedro Sánchez");
    strcpy(cliente3.correo, "pedro@email.com");  // ✅ CORREGIDO
    strcpy(cliente3.contrasena, "123456");
    strcpy(cliente3.telefono, "666345678");
    cliente3.tipo = USUARIO_CLIENTE;
    
    if (!usuario_crear(&cliente3)) {
        printf("❌ Error creando usuario Pedro\n");
        log_error("Error creando cliente3");
        return false;
    }
    printf("✅ Usuario Pedro creado (ID: %d) - Correo: %s\n", cliente3.id, cliente3.correo);
    
    // Usuario administrador adicional
    Usuario admin2 = {0};
    strcpy(admin2.nombre, "Ana Martínez");
    strcpy(admin2.correo, "ana@cinegestion.com");
    strcpy(admin2.contrasena, "admin456");
    strcpy(admin2.telefono, "666111222");
    admin2.tipo = USUARIO_ADMINISTRADOR;
    
    if (!usuario_crear(&admin2)) {
        printf("❌ Error creando admin Ana\n");
        log_error("Error creando admin2");
        return false;
    }
    printf("✅ Admin Ana creado (ID: %d) - Correo: %s\n", admin2.id, admin2.correo);
    
    printf("✅ Usuarios de prueba creados:\n");
    printf("   - juan@email.com / 123456 (Cliente)\n");
    printf("   - maria@email.com / 123456 (Cliente)\n");
    printf("   - pedro@email.com / 123456 (Cliente)\n");
    printf("   - ana@cinegestion.com / admin456 (Admin)\n");
    
    log_info("✅ Usuarios de prueba creados (3 clientes + 1 admin adicional)");
    return true;
}

// Crear películas de prueba
bool test_data_crear_peliculas() {
    printf("🎬 Creando películas de prueba...\n");
    log_info("Creando películas de prueba...");
    
    // Película 1
    Pelicula pelicula1 = {0};
    strcpy(pelicula1.titulo, "Avatar: El Camino del Agua");
    pelicula1.duracion = 192;
    strcpy(pelicula1.genero, "Ciencia Ficción");
    
    if (!pelicula_crear(&pelicula1)) {
        printf("❌ Error creando película Avatar\n");
        log_error("Error creando película 1");
        return false;
    }
    printf("✅ Película Avatar creada (ID: %d)\n", pelicula1.id);
    
    // Película 2
    Pelicula pelicula2 = {0};
    strcpy(pelicula2.titulo, "Top Gun: Maverick");
    pelicula2.duracion = 131;
    strcpy(pelicula2.genero, "Acción");
    
    if (!pelicula_crear(&pelicula2)) {
        printf("❌ Error creando película Top Gun\n");
        log_error("Error creando película 2");
        return false;
    }
    printf("✅ Película Top Gun creada (ID: %d)\n", pelicula2.id);
    
    // Película 3
    Pelicula pelicula3 = {0};
    strcpy(pelicula3.titulo, "El Gato con Botas: El Último Deseo");
    pelicula3.duracion = 102;
    strcpy(pelicula3.genero, "Animación");
    
    if (!pelicula_crear(&pelicula3)) {
        printf("❌ Error creando película El Gato con Botas\n");
        log_error("Error creando película 3");
        return false;
    }
    printf("✅ Película El Gato con Botas creada (ID: %d)\n", pelicula3.id);
    
    // Película 4
    Pelicula pelicula4 = {0};
    strcpy(pelicula4.titulo, "Scream VI");
    pelicula4.duracion = 123;
    strcpy(pelicula4.genero, "Terror");
    
    if (!pelicula_crear(&pelicula4)) {
        printf("❌ Error creando película Scream VI\n");
        log_error("Error creando película 4");
        return false;
    }
    printf("✅ Película Scream VI creada (ID: %d)\n", pelicula4.id);
    
    // Película 5
    Pelicula pelicula5 = {0};
    strcpy(pelicula5.titulo, "John Wick 4");
    pelicula5.duracion = 169;
    strcpy(pelicula5.genero, "Acción");
    
    if (!pelicula_crear(&pelicula5)) {
        printf("❌ Error creando película John Wick 4\n");
        log_error("Error creando película 5");
        return false;
    }
    printf("✅ Película John Wick 4 creada (ID: %d)\n", pelicula5.id);
    
    // Película 6
    Pelicula pelicula6 = {0};
    strcpy(pelicula6.titulo, "La Sirenita");
    pelicula6.duracion = 135;
    strcpy(pelicula6.genero, "Musical");
    
    if (!pelicula_crear(&pelicula6)) {
        printf("❌ Error creando película La Sirenita\n");
        log_error("Error creando película 6");
        return false;
    }
    printf("✅ Película La Sirenita creada (ID: %d)\n", pelicula6.id);
    
    printf("✅ Películas de prueba creadas (6 películas)\n");
    log_info("✅ Películas de prueba creadas (6 películas)");
    return true;
}

// Crear salas de prueba
bool test_data_crear_salas() {
    printf("🏛️  Creando salas de prueba...\n");
    log_info("Creando salas de prueba...");
    
    // Sala 1 - Sala pequeña
    Sala sala1 = {0};
    sala1.numero_asientos = 50;
    
    if (!sala_crear(&sala1)) {
        printf("❌ Error creando sala 1\n");
        log_error("Error creando sala 1");
        return false;
    }
    printf("✅ Sala 1 creada: %d asientos (ID: %d)\n", sala1.numero_asientos, sala1.id);
    
    // Sala 2 - Sala mediana
    Sala sala2 = {0};
    sala2.numero_asientos = 100;
    
    if (!sala_crear(&sala2)) {
        printf("❌ Error creando sala 2\n");
        log_error("Error creando sala 2");
        return false;
    }
    printf("✅ Sala 2 creada: %d asientos (ID: %d)\n", sala2.numero_asientos, sala2.id);
    
    // Sala 3 - Sala grande
    Sala sala3 = {0};
    sala3.numero_asientos = 150;
    
    if (!sala_crear(&sala3)) {
        printf("❌ Error creando sala 3\n");
        log_error("Error creando sala 3");
        return false;
    }
    printf("✅ Sala 3 creada: %d asientos (ID: %d)\n", sala3.numero_asientos, sala3.id);
    
    // Sala 4 - Sala VIP
    Sala sala4 = {0};
    sala4.numero_asientos = 30;
    
    if (!sala_crear(&sala4)) {
        printf("❌ Error creando sala 4\n");
        log_error("Error creando sala 4");
        return false;
    }
    printf("✅ Sala 4 VIP creada: %d asientos (ID: %d)\n", sala4.numero_asientos, sala4.id);
    
    printf("✅ Salas de prueba creadas (4 salas: 50, 100, 150, 30 asientos)\n");
    log_info("✅ Salas de prueba creadas (4 salas: 50, 100, 150, 30 asientos)");
    return true;
}

// Crear sesiones de prueba
bool test_data_crear_sesiones() {
    printf("📅 Creando sesiones de prueba...\n");
    log_info("Creando sesiones de prueba...");
    
    // Sesiones para Avatar (Película ID: 1)
    Sesion sesion1 = {0};
    sesion1.pelicula_id = 1;
    sesion1.sala_id = 3; // Sala grande
    strcpy(sesion1.hora_inicio, "2024-06-15 16:00:00");
    strcpy(sesion1.hora_fin, "2024-06-15 19:12:00");
    
    if (!sesion_crear(&sesion1)) {
        printf("❌ Error creando sesión 1 (Avatar 16:00)\n");
        log_error("Error creando sesión 1");
        return false;
    }
    printf("✅ Sesión Avatar 16:00 creada (ID: %d)\n", sesion1.id);
    
    Sesion sesion2 = {0};
    sesion2.pelicula_id = 1;
    sesion2.sala_id = 3;
    strcpy(sesion2.hora_inicio, "2024-06-15 20:00:00");
    strcpy(sesion2.hora_fin, "2024-06-15 23:12:00");
    
    if (!sesion_crear(&sesion2)) {
        printf("❌ Error creando sesión 2 (Avatar 20:00)\n");
        log_error("Error creando sesión 2");
        return false;
    }
    printf("✅ Sesión Avatar 20:00 creada (ID: %d)\n", sesion2.id);
    
    // Sesiones para Top Gun (Película ID: 2)
    Sesion sesion3 = {0};
    sesion3.pelicula_id = 2;
    sesion3.sala_id = 2;
    strcpy(sesion3.hora_inicio, "2024-06-15 17:30:00");
    strcpy(sesion3.hora_fin, "2024-06-15 19:41:00");
    
    if (!sesion_crear(&sesion3)) {
        printf("❌ Error creando sesión 3 (Top Gun 17:30)\n");
        log_error("Error creando sesión 3");
        return false;
    }
    printf("✅ Sesión Top Gun 17:30 creada (ID: %d)\n", sesion3.id);
    
    Sesion sesion4 = {0};
    sesion4.pelicula_id = 2;
    sesion4.sala_id = 2;
    strcpy(sesion4.hora_inicio, "2024-06-15 21:00:00");
    strcpy(sesion4.hora_fin, "2024-06-15 23:11:00");
    
    if (!sesion_crear(&sesion4)) {
        printf("❌ Error creando sesión 4 (Top Gun 21:00)\n");
        log_error("Error creando sesión 4");
        return false;
    }
    printf("✅ Sesión Top Gun 21:00 creada (ID: %d)\n", sesion4.id);
    
    // Sesiones para El Gato con Botas (Película ID: 3)
    Sesion sesion5 = {0};
    sesion5.pelicula_id = 3;
    sesion5.sala_id = 1;
    strcpy(sesion5.hora_inicio, "2024-06-15 16:30:00");
    strcpy(sesion5.hora_fin, "2024-06-15 18:12:00");
    
    if (!sesion_crear(&sesion5)) {
        printf("❌ Error creando sesión 5 (Gato con Botas 16:30)\n");
        log_error("Error creando sesión 5");
        return false;
    }
    printf("✅ Sesión Gato con Botas 16:30 creada (ID: %d)\n", sesion5.id);
    
    printf("✅ Sesiones de prueba creadas (5 sesiones iniciales)\n");
    log_info("✅ Sesiones de prueba creadas");
    return true;
}

// Crear algunas ventas de ejemplo
bool test_data_crear_ventas_ejemplo() {
    printf("💰 Intentando crear ventas de ejemplo...\n");
    log_info("Creando ventas de ejemplo...");
    
    // Por ahora, simplemente reportamos éxito sin crear ventas reales
    // ya que requieren billetes que dependen de asientos que pueden no existir
    printf("ℹ️  Ventas de ejemplo omitidas (requieren billetes y asientos)\n");
    log_info("Ventas de ejemplo omitidas");
    return true;
}

// Verificar si ya existen datos
bool test_data_verificar_datos_existentes() {
    printf("🔍 Verificando si ya existen datos...\n");
    
    // Verificar si hay películas
    Pelicula* peliculas = NULL;
    int num_peliculas = 0;
    
    if (pelicula_listar(&peliculas, &num_peliculas)) {
        if (num_peliculas > 0) {
            printf("ℹ️  Ya hay %d películas en la base de datos\n", num_peliculas);
            pelicula_liberar_lista(peliculas, num_peliculas);
            return true; // Ya hay datos
        }
        pelicula_liberar_lista(peliculas, num_peliculas);
    }
    
    printf("📝 No se encontraron datos existentes\n");
    return false; // No hay datos
}

// Limpiar todos los datos de prueba
bool test_data_limpiar() {
    printf("🗑️  Limpiando datos de prueba...\n");
    log_info("Limpiando datos de prueba...");
    
    // Eliminar en orden inverso para respetar las claves foráneas
    if (!db_execute("DELETE FROM Venta_Billetes;") ||
        !db_execute("DELETE FROM Venta;") ||
        !db_execute("DELETE FROM Billete;") ||
        !db_execute("DELETE FROM Sesion;") ||
        !db_execute("DELETE FROM Asiento;") ||
        !db_execute("DELETE FROM Sala;") ||
        !db_execute("DELETE FROM Pelicula;") ||
        !db_execute("DELETE FROM Usuarios WHERE TipoUsuario = 'Cliente';")) {
        
        printf("❌ Error limpiando datos de prueba\n");
        log_error("Error limpiando datos de prueba");
        return false;
    }
    
    printf("✅ Datos de prueba limpiados\n");
    log_info("✅ Datos de prueba limpiados");
    return true;
}