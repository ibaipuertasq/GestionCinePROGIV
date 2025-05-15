// bridge.cpp
#include "bridge.h"
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <iostream>

#include "../common/models/pelicula.h"
#include "../common/models/sesion.h"

// Incluir las cabeceras de C necesarias
extern "C" {
    #include "../../hito2/src/database.h"
    #include "../../hito2/src/models/pelicula.h"
    #include "../../hito2/src/models/sesion.h"
    #include "../../hito2/src/models/sala.h"
    #include "../../hito2/src/models/asiento.h"
    #include "../../hito2/src/models/billete.h"
    #include "../../hito2/src/models/venta.h"
    #include "../../hito2/src/models/usuario.h"
    #include "../../hito2/src/auth.h"
    #include "../../hito2/src/utils/logger.h"
}

// Inicialización y cierre
bool bridge_init_db(const char* db_path) {
    // Inicialización de log
    log_init("logs/server.log", LOG_INFO);
    log_info("Inicializando la base de datos: %s", db_path);
    
    // Inicialización de base de datos
    bool result = db_init(db_path);
    
    if (!result) {
        log_error("Error al inicializar la base de datos");
    } else {
        log_info("Base de datos inicializada correctamente");
    }
    
    // Inicialización de autenticación
    auth_init();
    
    return result;
}

void bridge_close_db() {
    db_close();
    log_close();
}

// Autenticación
int bridge_login(const char* email, const char* password) {
    Usuario usuario;
    if (usuario_autenticar(email, password, &usuario)) {
        return usuario.id;
    }
    return -1;
}

int bridge_user_get_type(int userId) {
    Usuario usuario;
    if (usuario_obtener_por_id(userId, &usuario)) {
        return static_cast<int>(usuario.tipo);
    }
    return -1;
}

std::string bridge_user_get_name(int userId) {
    Usuario usuario;
    if (usuario_obtener_por_id(userId, &usuario)) {
        return usuario.nombre;
    }
    return "";
}

bool bridge_user_is_admin(int userId) {
    return bridge_user_get_type(userId) == static_cast<int>(USUARIO_ADMINISTRADOR);
}

// Películas
bool bridge_pelicula_list(std::vector<Pelicula>* peliculas, int* num_peliculas) {
    Pelicula* c_peliculas = nullptr;
    int c_num_peliculas = 0;
    
    bool result = pelicula_listar(&c_peliculas, &c_num_peliculas);
    
    if (result && c_num_peliculas > 0) {
        peliculas->clear();
        
        for (int i = 0; i < c_num_peliculas; i++) {
            Pelicula pelicula;
            pelicula.setId(c_peliculas[i].id);
            pelicula.setTitulo(std::string(c_peliculas[i].titulo));
            pelicula.setDuracion(c_peliculas[i].duracion);
            pelicula.setGenero(std::string(c_peliculas[i].genero));

            peliculas->push_back(pelicula);
        }
        
        *num_peliculas = c_num_peliculas;
        pelicula_liberar_lista(c_peliculas, c_num_peliculas);
    } else {
        *num_peliculas = 0;
    }
    
    return result;
}

bool bridge_pelicula_get_by_id(int id, Pelicula* pelicula) {
    Pelicula c_pelicula;
    bool result = pelicula_obtener_por_id(id, &c_pelicula);
    
    if (result) {
        pelicula->setId(c_pelicula.id);
        pelicula->setTitulo(std::string(c_pelicula.titulo));
        pelicula->setDuracion(c_pelicula.duracion);
        pelicula->setGenero(std::string(c_pelicula.genero));
    }
    
    return result;
}

bool bridge_pelicula_create(Pelicula* pelicula) {
    Pelicula c_pelicula;
    c_pelicula.id = pelicula->getId();
    strncpy(c_pelicula.titulo, pelicula->getTitulo().c_str(), sizeof(c_pelicula.titulo) - 1);
    c_pelicula.duracion = pelicula->getDuracion();
    strncpy(c_pelicula.genero, pelicula->getGenero().c_str(), sizeof(c_pelicula.genero) - 1);
    
    bool result = pelicula_crear(&c_pelicula);
    
    if (result) {
        pelicula->setId(c_pelicula.id);
    }
    
    return result;
}

bool bridge_pelicula_update(Pelicula* pelicula) {
    Pelicula c_pelicula;
    c_pelicula.id = pelicula->getId();
    strncpy(c_pelicula.titulo, pelicula->getTitulo().c_str(), sizeof(c_pelicula.titulo) - 1);
    c_pelicula.duracion = pelicula->getDuracion();
    strncpy(c_pelicula.genero, pelicula->getGenero().c_str(), sizeof(c_pelicula.genero) - 1);
    
    return pelicula_actualizar(&c_pelicula);
}

bool bridge_pelicula_delete(int id) {
    return pelicula_eliminar(id);
}

bool bridge_pelicula_search_by_titulo(const char* titulo, std::vector<Pelicula>* peliculas, int* num_peliculas) {
    Pelicula* c_peliculas = nullptr;
    int c_num_peliculas = 0;
    
    bool result = pelicula_buscar_por_titulo(titulo, &c_peliculas, &c_num_peliculas);
    
    if (result && c_num_peliculas > 0) {
        peliculas->clear();
        
        for (int i = 0; i < c_num_peliculas; i++) {
            Pelicula pelicula(
                c_peliculas[i].id,
                c_peliculas[i].titulo,
                c_peliculas[i].duracion,
                c_peliculas[i].genero
            );
            
            peliculas->push_back(pelicula);
        }
        
        *num_peliculas = c_num_peliculas;
        pelicula_liberar_lista(c_peliculas, c_num_peliculas);
    } else {
        *num_peliculas = 0;
    }
    
    return result;
}

bool bridge_pelicula_search_by_genero(const char* genero, std::vector<Pelicula>* peliculas, int* num_peliculas) {
    Pelicula* c_peliculas = nullptr;
    int c_num_peliculas = 0;
    
    bool result = pelicula_buscar_por_genero(genero, &c_peliculas, &c_num_peliculas);
    
    if (result && c_num_peliculas > 0) {
        peliculas->clear();
        
        for (int i = 0; i < c_num_peliculas; i++) {
            Pelicula pelicula(
                c_peliculas[i].id,
                c_peliculas[i].titulo,
                c_peliculas[i].duracion,
                c_peliculas[i].genero
            );
            
            peliculas->push_back(pelicula);
        }
        
        *num_peliculas = c_num_peliculas;
        pelicula_liberar_lista(c_peliculas, c_num_peliculas);
    } else {
        *num_peliculas = 0;
    }
    
    return result;
}

// Sesiones
bool bridge_sesion_list(std::vector<Sesion>* sesiones, int* num_sesiones) {
    Sesion* c_sesiones = nullptr;
    int c_num_sesiones = 0;
    
    bool result = sesion_listar(&c_sesiones, &c_num_sesiones);
    
    if (result && c_num_sesiones > 0) {
        sesiones->clear();
        
        for (int i = 0; i < c_num_sesiones; i++) {
            Sesion sesion(
                c_sesiones[i].id,
                c_sesiones[i].pelicula_id,
                c_sesiones[i].sala_id,
                c_sesiones[i].hora_inicio,
                c_sesiones[i].hora_fin
            );
            
            sesiones->push_back(sesion);
        }
        
        *num_sesiones = c_num_sesiones;
        sesion_liberar_lista(c_sesiones, c_num_sesiones);
    } else {
        *num_sesiones = 0;
    }
    
    return result;
}

// Implementa el resto de funciones bridge de manera similar
// Por ejemplo:

bool bridge_sesion_get_by_id(int id, Sesion* sesion) {
    Sesion c_sesion;
    bool result = sesion_obtener_por_id(id, &c_sesion);
    
    if (result) {
        sesion->setId(c_sesion.id);
        sesion->setPeliculaId(c_sesion.pelicula_id);
        sesion->setSalaId(c_sesion.sala_id);
        sesion->setHoraInicio(c_sesion.hora_inicio);
        sesion->setHoraFin(c_sesion.hora_fin);
    }
    
    return result;
}

bool bridge_billete_esta_disponible(int sesion_id, int asiento_id) {
    return billete_esta_disponible(sesion_id, asiento_id);
}

int bridge_venta_create(int usuario_id, int* sesion_ids, int* asiento_ids, int num_billetes, double descuento) {
    // Crear los billetes
    Billete* billetes = (Billete*)malloc(num_billetes * sizeof(Billete));
    if (!billetes) {
        log_error("Error de memoria al crear billetes");
        return -1;
    }
    
    for (int i = 0; i < num_billetes; i++) {
        billetes[i].id = 0;  // Será asignado por la base de datos
        billetes[i].sesion_id = sesion_ids[i];
        billetes[i].asiento_id = asiento_ids[i];
        billetes[i].precio = billete_calcular_precio_base(sesion_ids[i]);
    }
    
    // Crear la venta
    Venta venta;
    venta.id = 0;  // Será asignado por la base de datos
    venta.usuario_id = usuario_id;
    venta.descuento = descuento;
    
    // La hora actual se establece en venta_crear
    venta.fecha[0] = '\0';
    
    // Crear la venta con los billetes
    if (venta_crear(&venta, billetes, num_billetes)) {
        free(billetes);
        return venta.id;
    }
    
    free(billetes);
    return -1;
}

// Continúa implementando el resto de funciones del bridge...