// sesion.cpp
#include "sesion.h"
#include <sstream>

Sesion::Sesion() : id(0), pelicula_id(0), sala_id(0) {}

Sesion::Sesion(int id, int pelicula_id, int sala_id, 
               const std::string& hora_inicio, const std::string& hora_fin)
    : id(id), pelicula_id(pelicula_id), sala_id(sala_id), 
      hora_inicio(hora_inicio), hora_fin(hora_fin) {}

int Sesion::getId() const {
    return id;
}

int Sesion::getPeliculaId() const {
    return pelicula_id;
}

int Sesion::getSalaId() const {
    return sala_id;
}

std::string Sesion::getHoraInicio() const {
    return hora_inicio;
}

std::string Sesion::getHoraFin() const {
    return hora_fin;
}

void Sesion::setId(int id) {
    this->id = id;
}

void Sesion::setPeliculaId(int pelicula_id) {
    this->pelicula_id = pelicula_id;
}

void Sesion::setSalaId(int sala_id) {
    this->sala_id = sala_id;
}

void Sesion::setHoraInicio(const std::string& hora_inicio) {
    this->hora_inicio = hora_inicio;
}

void Sesion::setHoraFin(const std::string& hora_fin) {
    this->hora_fin = hora_fin;
}

void Sesion::serialize(Message& msg) const {
    msg.addInt(id);
    msg.addInt(pelicula_id);
    msg.addInt(sala_id);
    msg.addString(hora_inicio);
    msg.addString(hora_fin);
}

Sesion Sesion::deserialize(Message& msg) {
    Sesion s;
    s.id = msg.getInt();
    s.pelicula_id = msg.getInt();
    s.sala_id = msg.getInt();
    s.hora_inicio = msg.getString();
    s.hora_fin = msg.getString();
    return s;
}

std::string Sesion::toString() const {
    std::stringstream ss;
    ss << "ID: " << id 
       << " | Película ID: " << pelicula_id 
       << " | Sala ID: " << sala_id 
       << " | Inicio: " << hora_inicio 
       << " | Fin: " << hora_fin;
    return ss.str();
}

std::vector<Sesion> deserializeSesionList(Message& msg) {
    std::vector<Sesion> result;
    int count = msg.getInt();
    
    for (int i = 0; i < count; i++) {
        result.push_back(Sesion::deserialize(msg));
    }
    
    return result;
}

void serializeSesionList(const std::vector<Sesion>& sesiones, Message& msg) {
    msg.addInt(sesiones.size());
    
    for (const auto& sesion : sesiones) {
        sesion.serialize(msg);
    }
}