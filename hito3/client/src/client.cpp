// client.cpp
#include "client.h"
#include <iostream>
#include <sstream>
#include <ws2tcpip.h>

Client::Client(const std::string& serverIp, int serverPort)
    : clientSocket(-1), serverIp(serverIp), serverPort(serverPort), 
      connected(false), loggedIn(false), userId(-1), userType(-1) {
}

Client::~Client() {
    disconnect();
}

bool Client::initializeWinsock() {
    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) {
        std::cerr << "WSAStartup failed: " << result << std::endl;
        lastError = "Error al inicializar Winsock";
        return false;
    }
    return true;
}

bool Client::connect() {
    if (connected) {
        return true;
    }
    
    // Inicializar Winsock
    if (!initializeWinsock()) {
        return false;
    }
    
    // Crear el socket
    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == INVALID_SOCKET) {
        std::cerr << "Error al crear el socket: " << WSAGetLastError() << std::endl;
        WSACleanup();
        lastError = "Error al crear el socket";
        return false;
    }
    
    // Configurar la dirección del servidor
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(serverPort);
    serverAddr.sin_addr.s_addr = inet_addr(serverIp.c_str());
    
    // Conectar al servidor
    if (::connect(clientSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "Error al conectar al servidor: " << WSAGetLastError() << std::endl;
        closesocket(clientSocket);
        WSACleanup();
        lastError = "Error al conectar al servidor";
        return false;
    }
    
    connected = true;
    return true;
}

void Client::disconnect() {
    if (connected) {
        closesocket(clientSocket);
        connected = false;
        WSACleanup();
    }
    
    // Limpiar información de sesión
    if (loggedIn) {
        logout();
    }
}

bool Client::isConnected() const {
    return connected;
}

bool Client::login(const std::string& email, const std::string& password) {
    if (!connected) {
        lastError = "No conectado al servidor";
        return false;
    }
    
    // Crear mensaje de solicitud
    Message request(OP_LOGIN);
    request.addString(email);
    request.addString(password);
    
    // Enviar solicitud y recibir respuesta
    Message response = sendRequest(OP_LOGIN, request);
    
    if (response.getOpCode() == OP_OK) {
        userId = response.getInt();
        userType = response.getInt();
        userName = response.getString();
        loggedIn = true;
        return true;
    } else {
        lastError = response.getData();
        return false;
    }
}

void Client::logout() {
    if (connected && loggedIn) {
        // Enviar solicitud de logout
        sendRequest(OP_LOGOUT);
    }
    
    // Limpiar información de sesión localmente
    loggedIn = false;
    userId = -1;
    userType = -1;
    userName = "";
}

bool Client::isLoggedIn() const {
    return loggedIn;
}

int Client::getUserId() const {
    return userId;
}

int Client::getUserType() const {
    return userType;
}

std::string Client::getUserName() const {
    return userName;
}

bool Client::isAdmin() const {
    return userType == 1; // 1 = USUARIO_ADMINISTRADOR
}

std::vector<Pelicula> Client::getPeliculas() {
    std::vector<Pelicula> result;
    
    if (!connected) {
        lastError = "No conectado al servidor";
        return result;
    }
    
    // Enviar solicitud y recibir respuesta
    Message response = sendRequest(OP_PELICULA_LIST);
    
    if (response.getOpCode() == OP_OK) {
        result = deserializePeliculaList(response);
    } else {
        lastError = response.getData();
    }
    
    return result;
}

Pelicula Client::getPelicula(int id) {
    Pelicula result;
    
    if (!connected) {
        lastError = "No conectado al servidor";
        return result;
    }
    
    // Crear mensaje de solicitud
    Message request(OP_PELICULA_GET);
    request.addInt(id);
    
    // Enviar solicitud y recibir respuesta
    Message response = sendRequest(OP_PELICULA_GET, request);
    
    if (response.getOpCode() == OP_OK) {
        result = Pelicula::deserialize(response);
    } else {
        lastError = response.getData();
    }
    
    return result;
}

bool Client::createPelicula(Pelicula& pelicula) {
    if (!connected) {
        lastError = "No conectado al servidor";
        return false;
    }
    
    if (!loggedIn || !isAdmin()) {
        lastError = "No tiene permisos para esta operación";
        return false;
    }
    
    // Crear mensaje de solicitud
    Message request(OP_PELICULA_CREATE);
    pelicula.serialize(request);
    
    // Enviar solicitud y recibir respuesta
    Message response = sendRequest(OP_PELICULA_CREATE, request);
    
    if (response.getOpCode() == OP_OK) {
        pelicula.setId(response.getInt());
        return true;
    } else {
        lastError = response.getData();
        return false;
    }
}

bool Client::updatePelicula(const Pelicula& pelicula) {
    if (!connected) {
        lastError = "No conectado al servidor";
        return false;
    }
    
    if (!loggedIn || !isAdmin()) {
        lastError = "No tiene permisos para esta operación";
        return false;
    }
    
    // Crear mensaje de solicitud
    Message request(OP_PELICULA_UPDATE);
    pelicula.serialize(request);
    
    // Enviar solicitud y recibir respuesta
    Message response = sendRequest(OP_PELICULA_UPDATE, request);
    
    if (response.getOpCode() == OP_OK) {
        return true;
    } else {
        lastError = response.getData();
        return false;
    }
}

bool Client::deletePelicula(int id) {
    if (!connected) {
        lastError = "No conectado al servidor";
        return false;
    }
    
    if (!loggedIn || !isAdmin()) {
        lastError = "No tiene permisos para esta operación";
        return false;
    }
    
    // Crear mensaje de solicitud
    Message request(OP_PELICULA_DELETE);
    request.addInt(id);
    
    // Enviar solicitud y recibir respuesta
    Message response = sendRequest(OP_PELICULA_DELETE, request);
    
    if (response.getOpCode() == OP_OK) {
        return true;
    } else {
        lastError = response.getData();
        return false;
    }
}

std::vector<Pelicula> Client::searchPeliculasByTitulo(const std::string& titulo) {
    std::vector<Pelicula> result;
    
    if (!connected) {
        lastError = "No conectado al servidor";
        return result;
    }
    
    // Crear mensaje de solicitud
    Message request(OP_PELICULA_SEARCH_TITULO);
    request.addString(titulo);
    
    // Enviar solicitud y recibir respuesta
    Message response = sendRequest(OP_PELICULA_SEARCH_TITULO, request);
    
    if (response.getOpCode() == OP_OK) {
        result = deserializePeliculaList(response);
    } else {
        lastError = response.getData();
    }
    
    return result;
}

std::vector<Pelicula> Client::searchPeliculasByGenero(const std::string& genero) {
    std::vector<Pelicula> result;
    
    if (!connected) {
        lastError = "No conectado al servidor";
        return result;
    }
    
    // Crear mensaje de solicitud
    Message request(OP_PELICULA_SEARCH_GENERO);
    request.addString(genero);
    
    // Enviar solicitud y recibir respuesta
    Message response = sendRequest(OP_PELICULA_SEARCH_GENERO, request);
    
    if (response.getOpCode() == OP_OK) {
        result = deserializePeliculaList(response);
    } else {
        lastError = response.getData();
    }
    
    return result;
}

// Implementar las funciones de Sesiones
std::vector<Sesion> Client::getSesiones() {
    std::vector<Sesion> result;
    
    if (!connected) {
        lastError = "No conectado al servidor";
        return result;
    }
    
    // Enviar solicitud y recibir respuesta
    Message response = sendRequest(OP_SESION_LIST);
    
    if (response.getOpCode() == OP_OK) {
        result = deserializeSesionList(response);
    } else {
        lastError = response.getData();
    }
    
    return result;
}

Sesion Client::getSesion(int id) {
    Sesion result;
    
    if (!connected) {
        lastError = "No conectado al servidor";
        return result;
    }
    
    // Crear mensaje de solicitud
    Message request(OP_SESION_GET);
    request.addInt(id);
    
    // Enviar solicitud y recibir respuesta
    Message response = sendRequest(OP_SESION_GET, request);
    
    if (response.getOpCode() == OP_OK) {
        result = Sesion::deserialize(response);
    } else {
        lastError = response.getData();
    }
    
    return result;
}

// Implementar las demás funciones de manera similar...

// Funciones de utilidad
Message Client::sendRequest(OperationCode opCode, const Message& request) {
    if (!connected) {
        return Message(OP_ERROR, "No conectado al servidor");
    }
    
    // Enviar la solicitud
    if (!sendMessage(clientSocket, request)) {
        lastError = "Error al enviar la solicitud";
        return Message(OP_ERROR, lastError);
    }
    
    // Recibir la respuesta
    Message response = receiveMessage(clientSocket);
    
    if (response.getOpCode() == OP_ERROR) {
        lastError = response.getData();
    }
    
    return response;
}

std::string Client::getLastError() const {
    return lastError;
}

// ... Continuar implementando el resto de métodos del cliente ...