// protocol.h
#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <string>
#include <vector>
#include <sstream>
#include <cstring>

// Códigos de operación para el protocolo
enum OperationCode {
    // Operaciones de autenticación
    OP_LOGIN = 100,
    OP_LOGOUT = 101,
    
    // Operaciones de películas
    OP_PELICULA_LIST = 200,
    OP_PELICULA_GET = 201,
    OP_PELICULA_CREATE = 202,
    OP_PELICULA_UPDATE = 203,
    OP_PELICULA_DELETE = 204,
    OP_PELICULA_SEARCH_TITULO = 205,
    OP_PELICULA_SEARCH_GENERO = 206,
    
    // Operaciones de sesiones
    OP_SESION_LIST = 300,
    OP_SESION_GET = 301,
    OP_SESION_CREATE = 302,
    OP_SESION_UPDATE = 303,
    OP_SESION_DELETE = 304,
    OP_SESION_SEARCH_PELICULA = 305,
    OP_SESION_SEARCH_SALA = 306,
    OP_SESION_SEARCH_FECHA = 307,
    
    // Operaciones de salas y asientos
    OP_SALA_LIST = 400,
    OP_SALA_GET = 401,
    OP_ASIENTO_LIST_BY_SALA = 402,
    
    // Operaciones de billetes y ventas
    OP_BILLETE_CREATE = 500,
    OP_BILLETE_DISPONIBILIDAD = 501,
    OP_VENTA_CREATE = 502,
    OP_VENTA_LIST_BY_USER = 503,
    OP_VENTA_GET = 504,
    OP_VENTA_GET_BILLETES = 505,
    
    // Respuestas y errores
    OP_OK = 900,
    OP_ERROR = 901
};

// Clase para mensajes del protocolo
class Message {
private:
    OperationCode opCode;
    std::string data;

public:
    Message(OperationCode code, const std::string& content = "");
    
    // Métodos para añadir datos al mensaje
    void addString(const std::string& str);
    void addInt(int value);
    void addDouble(double value);
    void addBool(bool value);
    
    // Métodos para leer datos del mensaje
    std::string getString();
    int getInt();
    double getDouble();
    bool getBool();
    
    // Métodos para serializar/deserializar
    std::string serialize() const;
    static Message deserialize(const std::string& data);
    
    // Getters
    OperationCode getOpCode() const;
    std::string getData() const;
    
    // Utilidades
    void clear();
    bool hasMoreData() const;
};

// Funciones de comunicación
bool sendMessage(int socket, const Message& msg);
Message receiveMessage(int socket);

// Constantes
const int BUFFER_SIZE = 4096;
const char SEPARATOR = '|';
const char END_MESSAGE = '\n';

#endif // PROTOCOL_H