// main.cpp
#include "client.h"
#include "menu.h"
#include <iostream>
#include <string>

int main() {
    std::cout << "=== CLIENTE DE GESTIÓN DE CINE ===" << std::endl;
    
    // Configuración del cliente
    std::string serverIp = "127.0.0.1"; // Localhost por defecto
    int serverPort = 8080;              // Puerto por defecto
    
    // Crear el cliente
    Client client(serverIp, serverPort);
    
    // Intentar conectar al servidor
    if (!client.connect()) {
        std::cerr << "Error al conectar al servidor: " << client.getLastError() << std::endl;
        std::cout << "Presione cualquier tecla para salir..." << std::endl;
        getchar();
        return 1;
    }
    
    std::cout << "Conectado al servidor exitosamente" << std::endl;
    
    // Crear e iniciar el menú
    Menu menu(&client);
    menu.ejecutar();
    
    // Desconectar del servidor
    client.disconnect();
    
    std::cout << "Gracias por usar CINE GESTIÓN" << std::endl;
    
    return 0;
}