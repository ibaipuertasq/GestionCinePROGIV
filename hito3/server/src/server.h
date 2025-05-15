// server.h
#ifndef SERVER_H
#define SERVER_H

#include <winsock2.h>
#include <ws2tcpip.h>
#include <string>
#include <thread> 
#include <vector>
#include <map>
#include <functional>
#include "../common/protocol.h"

class Server {
private:
    int serverSocket;
    int port;
    bool running;
    std::string dbPath;
    
    // Mapa de manejadores de operaciones
    std::map<OperationCode, std::function<Message(Message&, int)>> handlers;
    
    // Inicialización de WinSock
    bool initializeWinsock();
    
    // Inicializar los manejadores de operaciones
    void initializeHandlers();
    
    // Manejadores de operaciones específicas
    Message handleLogin(Message& request, int clientSocket);
    Message handleLogout(Message& request, int clientSocket);
    
    // Manejadores de películas
    Message handlePeliculaList(Message& request, int clientSocket);
    Message handlePeliculaGet(Message& request, int clientSocket);
    Message handlePeliculaCreate(Message& request, int clientSocket);
    Message handlePeliculaUpdate(Message& request, int clientSocket);
    Message handlePeliculaDelete(Message& request, int clientSocket);
    Message handlePeliculaSearchTitulo(Message& request, int clientSocket);
    Message handlePeliculaSearchGenero(Message& request, int clientSocket);
    
    // Manejadores de sesiones
    Message handleSesionList(Message& request, int clientSocket);
    Message handleSesionGet(Message& request, int clientSocket);
    Message handleSesionCreate(Message& request, int clientSocket);
    Message handleSesionUpdate(Message& request, int clientSocket);
    Message handleSesionDelete(Message& request, int clientSocket);
    Message handleSesionSearchPelicula(Message& request, int clientSocket);
    Message handleSesionSearchSala(Message& request, int clientSocket);
    Message handleSesionSearchFecha(Message& request, int clientSocket);
    
    // Manejadores de salas
    Message handleSalaList(Message& request, int clientSocket);
    Message handleSalaGet(Message& request, int clientSocket);
    Message handleAsientoListBySala(Message& request, int clientSocket);
    
    // Manejadores de billetes y ventas
    Message handleBilleteCreate(Message& request, int clientSocket);
    Message handleBilleteDisponibilidad(Message& request, int clientSocket);
    Message handleVentaCreate(Message& request, int clientSocket);
    Message handleVentaListByUser(Message& request, int clientSocket);
    Message handleVentaGet(Message& request, int clientSocket);
    Message handleVentaGetBilletes(Message& request, int clientSocket);
    
    // Registro de sesiones activas (ID cliente -> ID usuario)
    std::map<int, int> activeSessions;

public:
    Server(int port = 8080, const std::string& dbPath = "../../data/cine.db");
    ~Server();
    
    // Iniciar el servidor
    bool start();
    
    // Detener el servidor
    void stop();
    
    // Manejar un cliente
    void handleClient(int clientSocket);
    
    // Verificar si hay una sesión activa para un cliente
    bool isSessionActive(int clientSocket) const;
    
    // Obtener el ID de usuario de una sesión activa
    int getUserIdForSession(int clientSocket) const;
    
    // Crear una sesión activa
    void createSession(int clientSocket, int userId);
    
    // Eliminar una sesión activa
    void removeSession(int clientSocket);
};

#endif // SERVER_H