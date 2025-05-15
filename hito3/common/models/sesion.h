// sesion.h
#ifndef SESION_CPP_H
#define SESION_CPP_H

#include <string>
#include <vector>
#include "protocol.h"

class Sesion {
private:
    int id;
    int pelicula_id;
    int sala_id;
    std::string hora_inicio;
    std::string hora_fin;

public:
    // Constructores
    Sesion();
    Sesion(int id, int pelicula_id, int sala_id, 
           const std::string& hora_inicio, const std::string& hora_fin);
    
    // Getters
    int getId() const;
    int getPeliculaId() const;
    int getSalaId() const;
    std::string getHoraInicio() const;
    std::string getHoraFin() const;
    
    // Setters
    void setId(int id);
    void setPeliculaId(int pelicula_id);
    void setSalaId(int sala_id);
    void setHoraInicio(const std::string& hora_inicio);
    void setHoraFin(const std::string& hora_fin);
    
    // Serialización para el protocolo
    void serialize(Message& msg) const;
    static Sesion deserialize(Message& msg);
    
    // Impresión
    std::string toString() const;
};

// Funciones para trabajar con colecciones de sesiones
std::vector<Sesion> deserializeSesionList(Message& msg);
void serializeSesionList(const std::vector<Sesion>& sesiones, Message& msg);

#endif // SESION_CPP_H