// main.cpp (servidor)
#include "server.h"
#include <iostream>
#include <string>
#include <signal.h>

// Instancia global del servidor para poder cerrarlo en la señal de interrupción
Server* g_server = nullptr;

// Manejador de señales para un cierre limpio
void signalHandler(int signal) {
    if (g_server) {
        std::cout << "Recibida señal de interrupción, cerrando servidor..." << std::endl;
        g_server->stop();
    }
    exit(0);
}

int main() {
    std::cout << "=== SERVIDOR DE GESTIÓN DE CINE ===" << std::endl;
    
    // Configuración del servidor
    int port = 8080;
    std::string dbPath = "../../data/cine.db";
    
    // Configurar manejador de señales
    signal(SIGINT, signalHandler);
    
    // Crear e iniciar el servidor
    Server server(port, dbPath);
    g_server = &server;
    
    std::cout << "Iniciando servidor en puerto " << port << "..." << std::endl;
    
    if (!server.start()) {
        std::cerr << "Error al iniciar el servidor" << std::endl;
        return 1;
    }
    
    // El servidor se ejecuta en su propio bucle hasta que se detenga
    
    std::cout << "Servidor detenido" << std::endl;
    
    return 0;
}