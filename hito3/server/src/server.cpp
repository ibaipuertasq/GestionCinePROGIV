// server.cpp
#include "server.h"
#include "bridge.h"
#include "../common/models/pelicula.h"
#include "../common/models/sesion.h"
#include <iostream>
#include <sstream>
#include <thread>

Server::Server(int port, const std::string& dbPath) 
    : serverSocket(-1), port(port), running(false), dbPath(dbPath) {
    initializeHandlers();
}

Server::~Server() {
    stop();
}

bool Server::initializeWinsock() {
    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) {
        std::cerr << "WSAStartup failed: " << result << std::endl;
        return false;
    }
    return true;
}

void Server::initializeHandlers() {
    // Autenticación
    handlers[OP_LOGIN] = [this](Message& req, int client) { return handleLogin(req, client); };
    handlers[OP_LOGOUT] = [this](Message& req, int client) { return handleLogout(req, client); };
    
    // Películas
    handlers[OP_PELICULA_LIST] = [this](Message& req, int client) { return handlePeliculaList(req, client); };
    handlers[OP_PELICULA_GET] = [this](Message& req, int client) { return handlePeliculaGet(req, client); };
    handlers[OP_PELICULA_CREATE] = [this](Message& req, int client) { return handlePeliculaCreate(req, client); };
    handlers[OP_PELICULA_UPDATE] = [this](Message& req, int client) { return handlePeliculaUpdate(req, client); };
    handlers[OP_PELICULA_DELETE] = [this](Message& req, int client) { return handlePeliculaDelete(req, client); };
    handlers[OP_PELICULA_SEARCH_TITULO] = [this](Message& req, int client) { return handlePeliculaSearchTitulo(req, client); };
    handlers[OP_PELICULA_SEARCH_GENERO] = [this](Message& req, int client) { return handlePeliculaSearchGenero(req, client); };
    
    // Sesiones
    handlers[OP_SESION_LIST] = [this](Message& req, int client) { return handleSesionList(req, client); };
    handlers[OP_SESION_GET] = [this](Message& req, int client) { return handleSesionGet(req, client); };
    handlers[OP_SESION_CREATE] = [this](Message& req, int client) { return handleSesionCreate(req, client); };
    handlers[OP_SESION_UPDATE] = [this](Message& req, int client) { return handleSesionUpdate(req, client); };
    handlers[OP_SESION_DELETE] = [this](Message& req, int client) { return handleSesionDelete(req, client); };
    handlers[OP_SESION_SEARCH_PELICULA] = [this](Message& req, int client) { return handleSesionSearchPelicula(req, client); };
    handlers[OP_SESION_SEARCH_SALA] = [this](Message& req, int client) { return handleSesionSearchSala(req, client); };
    handlers[OP_SESION_SEARCH_FECHA] = [this](Message& req, int client) { return handleSesionSearchFecha(req, client); };
    
    // Salas y asientos
    handlers[OP_SALA_LIST] = [this](Message& req, int client) { return handleSalaList(req, client); };
    handlers[OP_SALA_GET] = [this](Message& req, int client) { return handleSalaGet(req, client); };
    handlers[OP_ASIENTO_LIST_BY_SALA] = [this](Message& req, int client) { return handleAsientoListBySala(req, client); };
    
    // Billetes y ventas
    handlers[OP_BILLETE_CREATE] = [this](Message& req, int client) { return handleBilleteCreate(req, client); };
    handlers[OP_BILLETE_DISPONIBILIDAD] = [this](Message& req, int client) { return handleBilleteDisponibilidad(req, client); };
    handlers[OP_VENTA_CREATE] = [this](Message& req, int client) { return handleVentaCreate(req, client); };
    handlers[OP_VENTA_LIST_BY_USER] = [this](Message& req, int client) { return handleVentaListByUser(req, client); };
    handlers[OP_VENTA_GET] = [this](Message& req, int client) { return handleVentaGet(req, client); };
    handlers[OP_VENTA_GET_BILLETES] = [this](Message& req, int client) { return handleVentaGetBilletes(req, client); };
}

bool Server::start() {
    // Inicializar Winsock
    if (!initializeWinsock()) {
        return false;
    }
    
    // Inicializar la base de datos
    if (!bridge_init_db(dbPath.c_str())) {
        std::cerr << "Error al inicializar la base de datos" << std::endl;
        return false;
    }
    
    // Crear el socket del servidor
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET) {
        std::cerr << "Error al crear el socket: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return false;
    }
    
    // Configurar la dirección del servidor
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    
    // Enlazar el socket
    if (bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "Error al enlazar el socket: " << WSAGetLastError() << std::endl;
        closesocket(serverSocket);
        WSACleanup();
        return false;
    }
    
    // Poner el socket en escucha
    if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR) {
        std::cerr << "Error al poner el socket en escucha: " << WSAGetLastError() << std::endl;
        closesocket(serverSocket);
        WSACleanup();
        return false;
    }
    
    std::cout << "Servidor iniciado en puerto " << port << std::endl;
    running = true;
    
    // Bucle principal para aceptar conexiones
    while (running) {
        // Aceptar una conexión entrante
        sockaddr_in clientAddr;
        int clientAddrSize = sizeof(clientAddr);
        int clientSocket = accept(serverSocket, (sockaddr*)&clientAddr, &clientAddrSize);
        
        if (clientSocket == INVALID_SOCKET) {
            std::cerr << "Error al aceptar conexión: " << WSAGetLastError() << std::endl;
            continue;
        }
        
        // Obtener información del cliente
        char* clientIP = inet_ntoa(clientAddr.sin_addr);
        std::cout << "Nueva conexión desde " << clientIP << ":" << ntohs(clientAddr.sin_port) << std::endl;
        
        // Crear un hilo para manejar al cliente
        handleClient(clientSocket);
    }
    
    return true;
}

void Server::stop() {
    running = false;
    
    if (serverSocket != INVALID_SOCKET) {
        closesocket(serverSocket);
        serverSocket = INVALID_SOCKET;
    }
    
    // Limpiar Winsock
    WSACleanup();
    
    // Cerrar la base de datos
    bridge_close_db();
    
    std::cout << "Servidor detenido" << std::endl;
}

void Server::handleClient(int clientSocket) {
    while (running) {
        Message request = receiveMessage(clientSocket);
        
        if (request.getOpCode() == OP_ERROR) {
            std::cout << "Error al recibir mensaje o conexión cerrada" << std::endl;
            break;
        }
        
        // Buscar el manejador para el código de operación
        auto it = handlers.find(request.getOpCode());
        if (it == handlers.end()) {
            // No hay manejador para esta operación
            Message response(OP_ERROR, "Operación no soportada");
            sendMessage(clientSocket, response);
            continue;
        }
        
        // Ejecutar el manejador
        Message response = it->second(request, clientSocket);
        
        // Enviar la respuesta
        if (!sendMessage(clientSocket, response)) {
            std::cout << "Error al enviar respuesta" << std::endl;
            break;
        }
    }
    
    // Cerrar la sesión y el socket
    removeSession(clientSocket);
    closesocket(clientSocket);
    std::cout << "Conexión cerrada (socket " << clientSocket << ")" << std::endl;
}

bool Server::isSessionActive(int clientSocket) const {
    return activeSessions.find(clientSocket) != activeSessions.end();
}

// server.cpp (continuación)
int Server::getUserIdForSession(int clientSocket) const {
    auto it = activeSessions.find(clientSocket);
    if (it != activeSessions.end()) {
        return it->second;
    }
    return -1;
}

void Server::createSession(int clientSocket, int userId) {
    activeSessions[clientSocket] = userId;
}

void Server::removeSession(int clientSocket) {
    activeSessions.erase(clientSocket);
}

// Implementación de los manejadores de operaciones

Message Server::handleLogin(Message& request, int clientSocket) {
    std::string email = request.getString();
    std::string password = request.getString();
    
    int userId = bridge_login(email.c_str(), password.c_str());
    
    if (userId > 0) {
        // Login exitoso
        createSession(clientSocket, userId);
        
        Message response(OP_OK);
        response.addInt(userId);
        
        // Obtener información del usuario
        int tipo = bridge_user_get_type(userId);
        std::string nombre = bridge_user_get_name(userId);
        
        response.addInt(tipo);
        response.addString(nombre);
        
        return response;
    } else {
        // Login fallido
        return Message(OP_ERROR, "Credenciales incorrectas");
    }
}

Message Server::handleLogout(Message& request, int clientSocket) {
    removeSession(clientSocket);
    return Message(OP_OK);
}

Message Server::handlePeliculaList(Message& request, int clientSocket) {
    std::vector<Pelicula> peliculas;
    int numPeliculas = 0;
    
    if (bridge_pelicula_list(&peliculas, &numPeliculas)) {
        Message response(OP_OK);
        serializePeliculaList(peliculas, response);
        return response;
    } else {
        return Message(OP_ERROR, "Error al listar películas");
    }
}

Message Server::handlePeliculaGet(Message& request, int clientSocket) {
    int id = request.getInt();
    
    Pelicula pelicula;
    if (bridge_pelicula_get_by_id(id, &pelicula)) {
        Message response(OP_OK);
        pelicula.serialize(response);
        return response;
    } else {
        return Message(OP_ERROR, "Película no encontrada");
    }
}

Message Server::handlePeliculaCreate(Message& request, int clientSocket) {
    if (!isSessionActive(clientSocket)) {
        return Message(OP_ERROR, "No hay sesión activa");
    }
    
    // Verificar si el usuario es administrador
    int userId = getUserIdForSession(clientSocket);
    if (!bridge_user_is_admin(userId)) {
        return Message(OP_ERROR, "No tiene permisos para esta operación");
    }
    
    Pelicula pelicula = Pelicula::deserialize(request);
    
    if (bridge_pelicula_create(&pelicula)) {
        Message response(OP_OK);
        response.addInt(pelicula.getId());
        return response;
    } else {
        return Message(OP_ERROR, "Error al crear película");
    }
}

Message Server::handlePeliculaUpdate(Message& request, int clientSocket) {
    if (!isSessionActive(clientSocket)) {
        return Message(OP_ERROR, "No hay sesión activa");
    }
    
    // Verificar si el usuario es administrador
    int userId = getUserIdForSession(clientSocket);
    if (!bridge_user_is_admin(userId)) {
        return Message(OP_ERROR, "No tiene permisos para esta operación");
    }
    
    Pelicula pelicula = Pelicula::deserialize(request);
    
    if (bridge_pelicula_update(&pelicula)) {
        return Message(OP_OK);
    } else {
        return Message(OP_ERROR, "Error al actualizar película");
    }
}

Message Server::handlePeliculaDelete(Message& request, int clientSocket) {
    if (!isSessionActive(clientSocket)) {
        return Message(OP_ERROR, "No hay sesión activa");
    }
    
    // Verificar si el usuario es administrador
    int userId = getUserIdForSession(clientSocket);
    if (!bridge_user_is_admin(userId)) {
        return Message(OP_ERROR, "No tiene permisos para esta operación");
    }
    
    int id = request.getInt();
    
    if (bridge_pelicula_delete(id)) {
        return Message(OP_OK);
    } else {
        return Message(OP_ERROR, "Error al eliminar película");
    }
}

Message Server::handlePeliculaSearchTitulo(Message& request, int clientSocket) {
    std::string titulo = request.getString();
    
    std::vector<Pelicula> peliculas;
    int numPeliculas = 0;
    
    if (bridge_pelicula_search_by_titulo(titulo.c_str(), &peliculas, &numPeliculas)) {
        Message response(OP_OK);
        serializePeliculaList(peliculas, response);
        return response;
    } else {
        return Message(OP_ERROR, "Error en la búsqueda");
    }
}

Message Server::handlePeliculaSearchGenero(Message& request, int clientSocket) {
    std::string genero = request.getString();
    
    std::vector<Pelicula> peliculas;
    int numPeliculas = 0;
    
    if (bridge_pelicula_search_by_genero(genero.c_str(), &peliculas, &numPeliculas)) {
        Message response(OP_OK);
        serializePeliculaList(peliculas, response);
        return response;
    } else {
        return Message(OP_ERROR, "Error en la búsqueda");
    }
}

Message Server::handleSesionList(Message& request, int clientSocket) {
    std::vector<Sesion> sesiones;
    int numSesiones = 0;
    
    if (bridge_sesion_list(&sesiones, &numSesiones)) {
        Message response(OP_OK);
        serializeSesionList(sesiones, response);
        return response;
    } else {
        return Message(OP_ERROR, "Error al listar sesiones");
    }
}

Message Server::handleSesionGet(Message& request, int clientSocket) {
    int id = request.getInt();
    
    Sesion sesion;
    if (bridge_sesion_get_by_id(id, &sesion)) {
        Message response(OP_OK);
        sesion.serialize(response);
        return response;
    } else {
        return Message(OP_ERROR, "Sesión no encontrada");
    }
}

Message Server::handleSesionCreate(Message& request, int clientSocket) {
    if (!isSessionActive(clientSocket)) {
        return Message(OP_ERROR, "No hay sesión activa");
    }
    
    // Verificar si el usuario es administrador
    int userId = getUserIdForSession(clientSocket);
    if (!bridge_user_is_admin(userId)) {
        return Message(OP_ERROR, "No tiene permisos para esta operación");
    }
    
    Sesion sesion = Sesion::deserialize(request);
    
    if (bridge_sesion_create(&sesion)) {
        Message response(OP_OK);
        response.addInt(sesion.getId());
        return response;
    } else {
        return Message(OP_ERROR, "Error al crear sesión");
    }
}

Message Server::handleSesionUpdate(Message& request, int clientSocket) {
    if (!isSessionActive(clientSocket)) {
        return Message(OP_ERROR, "No hay sesión activa");
    }
    
    // Verificar si el usuario es administrador
    int userId = getUserIdForSession(clientSocket);
    if (!bridge_user_is_admin(userId)) {
        return Message(OP_ERROR, "No tiene permisos para esta operación");
    }
    
    Sesion sesion = Sesion::deserialize(request);
    
    if (bridge_sesion_update(&sesion)) {
        return Message(OP_OK);
    } else {
        return Message(OP_ERROR, "Error al actualizar sesión");
    }
}

Message Server::handleSesionDelete(Message& request, int clientSocket) {
    if (!isSessionActive(clientSocket)) {
        return Message(OP_ERROR, "No hay sesión activa");
    }
    
    // Verificar si el usuario es administrador
    int userId = getUserIdForSession(clientSocket);
    if (!bridge_user_is_admin(userId)) {
        return Message(OP_ERROR, "No tiene permisos para esta operación");
    }
    
    int id = request.getInt();
    
    if (bridge_sesion_delete(id)) {
        return Message(OP_OK);
    } else {
        return Message(OP_ERROR, "Error al eliminar sesión");
    }
}

Message Server::handleSesionSearchPelicula(Message& request, int clientSocket) {
    int peliculaId = request.getInt();
    
    std::vector<Sesion> sesiones;
    int numSesiones = 0;
    
    if (bridge_sesion_search_by_pelicula(peliculaId, &sesiones, &numSesiones)) {
        Message response(OP_OK);
        serializeSesionList(sesiones, response);
        return response;
    } else {
        return Message(OP_ERROR, "Error en la búsqueda");
    }
}

Message Server::handleSesionSearchSala(Message& request, int clientSocket) {
    int salaId = request.getInt();
    
    std::vector<Sesion> sesiones;
    int numSesiones = 0;
    
    if (bridge_sesion_search_by_sala(salaId, &sesiones, &numSesiones)) {
        Message response(OP_OK);
        serializeSesionList(sesiones, response);
        return response;
    } else {
        return Message(OP_ERROR, "Error en la búsqueda");
    }
}

Message Server::handleSesionSearchFecha(Message& request, int clientSocket) {
    std::string fecha = request.getString();
    
    std::vector<Sesion> sesiones;
    int numSesiones = 0;
    
    if (bridge_sesion_search_by_fecha(fecha.c_str(), &sesiones, &numSesiones)) {
        Message response(OP_OK);
        serializeSesionList(sesiones, response);
        return response;
    } else {
        return Message(OP_ERROR, "Error en la búsqueda");
    }
}

// Implementa el resto de los manejadores de manera similar
// Aquí se muestran algunos ejemplos adicionales más complejos:

Message Server::handleBilleteDisponibilidad(Message& request, int clientSocket) {
    int sesionId = request.getInt();
    int asientoId = request.getInt();
    
    bool disponible = bridge_billete_esta_disponible(sesionId, asientoId);
    
    Message response(OP_OK);
    response.addBool(disponible);
    return response;
}

Message Server::handleVentaCreate(Message& request, int clientSocket) {
    if (!isSessionActive(clientSocket)) {
        return Message(OP_ERROR, "No hay sesión activa");
    }
    
    int userId = getUserIdForSession(clientSocket);
    
    // Leer los datos de la venta
    int numBilletes = request.getInt();
    std::vector<int> sesionIds(numBilletes);
    std::vector<int> asientoIds(numBilletes);
    
    for (int i = 0; i < numBilletes; i++) {
        sesionIds[i] = request.getInt();
        asientoIds[i] = request.getInt();
    }
    
    double descuento = request.getDouble();
    
    int ventaId = bridge_venta_create(userId, sesionIds.data(), asientoIds.data(), numBilletes, descuento);
    
    if (ventaId > 0) {
        Message response(OP_OK);
        response.addInt(ventaId);
        return response;
    } else {
        return Message(OP_ERROR, "Error al crear la venta");
    }
}

Message Server::handleVentaListByUser(Message& request, int clientSocket) {
    if (!isSessionActive(clientSocket)) {
        return Message(OP_ERROR, "No hay sesión activa");
    }
    
    int userId = getUserIdForSession(clientSocket);
    
    // Obtener las ventas del usuario
    std::vector<int> ventaIds;
    std::vector<std::string> fechas;
    std::vector<double> totales;
    int numVentas = 0;
    
    if (bridge_venta_list_by_user(userId, &ventaIds, &fechas, &totales, &numVentas)) {
        Message response(OP_OK);
        
        response.addInt(numVentas);
        
        for (int i = 0; i < numVentas; i++) {
            response.addInt(ventaIds[i]);
            response.addString(fechas[i]);
            response.addDouble(totales[i]);
        }
        
        return response;
    } else {
        return Message(OP_ERROR, "Error al obtener las ventas");
    }
}