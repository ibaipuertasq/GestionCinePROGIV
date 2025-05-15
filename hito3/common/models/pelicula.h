// pelicula.h
#ifndef PELICULA_CPP_H
#define PELICULA_CPP_H

#include <string>
#include <vector>
#include "protocol.h"

class Pelicula {
private:
    int id;
    std::string titulo;
    int duracion;
    std::string genero;

public:
    // Constructores
    Pelicula();
    Pelicula(int id, const std::string& titulo, int duracion, const std::string& genero);
    
    // Getters
    int getId() const;
    std::string getTitulo() const;
    int getDuracion() const;
    std::string getGenero() const;
    
    // Setters
    void setId(int id);
    void setTitulo(const std::string& titulo);
    void setDuracion(int duracion);
    void setGenero(const std::string& genero);
    
    // Serialización para el protocolo
    void serialize(Message& msg) const;
    static Pelicula deserialize(Message& msg);
    
    // Impresión
    std::string toString() const;
};

// Funciones para trabajar con colecciones de películas
std::vector<Pelicula> deserializePeliculaList(Message& msg);
void serializePeliculaList(const std::vector<Pelicula>& peliculas, Message& msg);

#endif // PELICULA_CPP_H