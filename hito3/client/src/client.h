// client.h
#ifndef CLIENT_H
#define CLIENT_H

#include <winsock2.h>
#include <ws2tcpip.h>
#include <string>
#include <vector>
#include "../common/protocol.h"
#include "../common/models/pelicula.h"
#include "../common/models/sesion.h"

class Client {
private:
    int clientSocket;
    std::string serverIp;
    int serverPort;
    bool connected;
    
    // Información de la sesión
    bool loggedIn;
    int userId;
    int userType;
    std::string userName;
    
    // Inicialización de WinSock
    bool initializeWinsock();

public:
    Client(const std::string& serverIp = "127.0.0.1", int serverPort = 8080);
    ~Client();
    
    // Conexión
    bool connect();
    void disconnect();
    bool isConnected() const;
    
    // Sesión
    bool login(const std::string& email, const std::string& password);
    void logout();
    bool isLoggedIn() const;
    int getUserId() const;
    int getUserType() const;
    std::string getUserName() const;
    bool isAdmin() const;
    
    // Películas
    std::vector<Pelicula> getPeliculas();
    Pelicula getPelicula(int id);
    bool createPelicula(Pelicula& pelicula);
    bool updatePelicula(const Pelicula& pelicula);
    bool deletePelicula(int id);
    std::vector<Pelicula> searchPeliculasByTitulo(const std::string& titulo);
    std::vector<Pelicula> searchPeliculasByGenero(const std::string& genero);
    
    // Sesiones
    // Sesiones
    std::vector<Sesion> getSesiones();
    Sesion getSesion(int id);
    bool createSesion(Sesion& sesion);
    bool updateSesion(const Sesion& sesion);
    bool deleteSesion(int id);
    std::vector<Sesion> getSesionesByPelicula(int peliculaId);
    std::vector<Sesion> getSesionesBySala(int salaId);
    std::vector<Sesion> getSesionesByFecha(const std::string& fecha);
    
    // Salas y asientos
    struct Sala {
        int id;
        int numAsientos;
    };
    
    struct Asiento {
        int id;
        int numero;
        bool disponible;
    };
    
    std::vector<Sala> getSalas();
    Sala getSala(int id);
    std::vector<Asiento> getAsientosBySala(int salaId);
    
    // Billetes y ventas
    struct Venta {
        int id;
        std::string fecha;
        double total;
    };
    
    struct VentaDetalle {
        int id;
        int usuarioId;
        std::string fecha;
        double descuento;
        double total;
        std::vector<std::pair<int, int>> billetes; // pares (sesionId, asientoId)
    };
    
    bool createBillete(int sesionId, int asientoId, double precio);
    bool checkAsientoDisponible(int sesionId, int asientoId);
    int createVenta(const std::vector<std::pair<int, int>>& billetes, double descuento = 0.0);
    std::vector<Venta> getVentasByUser();
    VentaDetalle getVentaDetalle(int ventaId);
    
    // Mensajes de error
    std::string getLastError() const;

private:
    std::string lastError;
    
    // Métodos auxiliares para comunicación
    Message sendRequest(OperationCode opCode, const Message& request = Message(OP_OK));
};

#endif // CLIENT_H