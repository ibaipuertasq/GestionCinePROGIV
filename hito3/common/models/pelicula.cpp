// pelicula.cpp
#include "pelicula.h"
#include <sstream>

Pelicula::Pelicula() : id(0), duracion(0) {}

Pelicula::Pelicula(int id, const std::string& titulo, int duracion, const std::string& genero)
    : id(id), titulo(titulo), duracion(duracion), genero(genero) {}

int Pelicula::getId() const {
    return id;
}

std::string Pelicula::getTitulo() const {
    return titulo;
}

int Pelicula::getDuracion() const {
    return duracion;
}

std::string Pelicula::getGenero() const {
    return genero;
}

void Pelicula::setId(int id) {
    this->id = id;
}

void Pelicula::setTitulo(const std::string& titulo) {
    this->titulo = titulo;
}

void Pelicula::setDuracion(int duracion) {
    this->duracion = duracion;
}

void Pelicula::setGenero(const std::string& genero) {
    this->genero = genero;
}

void Pelicula::serialize(Message& msg) const {
    msg.addInt(id);
    msg.addString(titulo);
    msg.addInt(duracion);
    msg.addString(genero);
}

Pelicula Pelicula::deserialize(Message& msg) {
    Pelicula p;
    p.id = msg.getInt();
    p.titulo = msg.getString();
    p.duracion = msg.getInt();
    p.genero = msg.getString();
    return p;
}

std::string Pelicula::toString() const {
    std::stringstream ss;
    ss << "ID: " << id << " | Título: " << titulo 
       << " | Duración: " << duracion << " min | Género: " << genero;
    return ss.str();
}

std::vector<Pelicula> deserializePeliculaList(Message& msg) {
    std::vector<Pelicula> result;
    int count = msg.getInt();
    
    for (int i = 0; i < count; i++) {
        result.push_back(Pelicula::deserialize(msg));
    }
    
    return result;
}

void serializePeliculaList(const std::vector<Pelicula>& peliculas, Message& msg) {
    msg.addInt(peliculas.size());
    
    for (const auto& pelicula : peliculas) {
        pelicula.serialize(msg);
    }
}