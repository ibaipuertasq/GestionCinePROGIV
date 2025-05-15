// menu.cpp
#include "menu.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <cmath>
#include <cstdlib>
#include <conio.h>

Menu::Menu(Client* client) : client(client), active(true) {
}

Menu::~Menu() {
}

void Menu::limpiarPantalla() {
    system("cls");
}

void Menu::mostrarEncabezado(const std::string& titulo) {
    int ancho = 60;
    int longitud_titulo = titulo.length();
    int padding = (ancho - longitud_titulo - 2) / 2;
    
    std::cout << std::endl;
    
    // Línea superior
    std::cout << "+";
    for (int i = 0; i < ancho - 2; i++) {
        std::cout << "-";
    }
    std::cout << "+" << std::endl;
    
    // Título
    std::cout << "|";
    for (int i = 0; i < padding; i++) {
        std::cout << " ";
    }
    std::cout << " " << titulo << " ";
    for (int i = 0; i < padding; i++) {
        std::cout << " ";
    }
    // Ajustar si la longitud del título es impar
    if ((ancho - longitud_titulo - 2) % 2 != 0) {
        std::cout << " ";
    }
    std::cout << "|" << std::endl;
    
    // Línea inferior
    std::cout << "+";
    for (int i = 0; i < ancho - 2; i++) {
        std::cout << "-";
    }
    std::cout << "+" << std::endl << std::endl;
}

void Menu::mostrarPiePagina() {
    int ancho = 60;
    
    std::cout << std::endl;
    std::cout << "+";
    for (int i = 0; i < ancho - 2; i++) {
        std::cout << "-";
    }
    std::cout << "+" << std::endl;
    
    // Información del usuario
    if (client->isLoggedIn()) {
        std::cout << "| Usuario: " << std::left << std::setw(47) << client->getUserName() << " |" << std::endl;
        std::cout << "| Tipo: " << std::left << std::setw(50) << (client->isAdmin() ? "Administrador" : "Cliente") << " |" << std::endl;
    }
    
    std::cout << "+";
    for (int i = 0; i < ancho - 2; i++) {
        std::cout << "-";
    }
    std::cout << "+" << std::endl;
}

void Menu::mostrarError(const std::string& mensaje) {
    std::cout << std::endl << "[ERROR] " << mensaje << std::endl;
}

void Menu::mostrarExito(const std::string& mensaje) {
    std::cout << std::endl << "[OK] " << mensaje << std::endl;
}

void Menu::pausar() {
    std::cout << std::endl << "Presione cualquier tecla para continuar...";
    _getch();
}

std::string Menu::leerTexto(const std::string& prompt) {
    std::string texto;
    std::cout << prompt << ": ";
    std::getline(std::cin, texto);
    
    // Si está vacío, volver a pedir
    if (texto.empty()) {
        return leerTexto(prompt);
    }
    
    return texto;
}

int Menu::leerEntero(const std::string& prompt, int min, int max) {
    std::string buffer;
    int valor;
    
    while (true) {
        std::cout << prompt << " (" << min << "-" << max << "): ";
        std::getline(std::cin, buffer);
        
        try {
            valor = std::stoi(buffer);
            if (valor >= min && valor <= max) {
                return valor;
            }
        } catch (...) {
            // Error de conversión, ignorar
        }
        
        mostrarError("Valor inválido. Intente de nuevo.");
    }
}

double Menu::leerDecimal(const std::string& prompt, double min, double max) {
    std::string buffer;
    double valor;
    
    while (true) {
        std::cout << prompt << " (" << min << "-" << max << "): ";
        std::getline(std::cin, buffer);
        
        try {
            valor = std::stod(buffer);
            if (valor >= min && valor <= max) {
                return valor;
            }
        } catch (...) {
            // Error de conversión, ignorar
        }
        
        mostrarError("Valor inválido. Intente de nuevo.");
    }
}

bool Menu::confirmar(const std::string& prompt) {
    std::string respuesta;
    
    std::cout << prompt << " (S/N): ";
    std::getline(std::cin, respuesta);
    
    // Convertir a minúsculas
    for (char& c : respuesta) {
        c = tolower(c);
    }
    
    return (respuesta == "s" || 
            respuesta == "si" || 
            respuesta == "sí" || 
            respuesta == "y" || 
            respuesta == "yes");
}

void Menu::mostrarMenuPrincipal() {
    limpiarPantalla();
    mostrarEncabezado("SISTEMA DE GESTIÓN DE CINE");
    
    std::cout << "1. Iniciar sesión" << std::endl;
    std::cout << "2. Salir" << std::endl;
    
    int opcion = leerEntero("Seleccione una opción", 1, 2);
    
    switch (opcion) {
        case 1:
            mostrar(MENU_AUTENTICACION);
            break;
        case 2:
            active = false;
            break;
    }
}

void Menu::mostrarMenuAutenticacion() {
    limpiarPantalla();
    mostrarEncabezado("INICIAR SESIÓN");
    
    std::string correo = leerTexto("Correo electrónico");
    std::string contrasena = leerTexto("Contraseña");
    
    if (client->login(correo, contrasena)) {
        mostrarExito("Inicio de sesión exitoso");
        
        if (client->isAdmin()) {
            mostrar(MENU_ADMIN);
        } else {
            mostrar(MENU_CLIENTE);
        }
    } else {
        mostrarError(client->getLastError());
        pausar();
        mostrar(MENU_PRINCIPAL);
    }
}

void Menu::mostrarMenuAdmin() {
    while (client->isLoggedIn() && client->isAdmin()) {
        limpiarPantalla();
        mostrarEncabezado("MENÚ DE ADMINISTRADOR");
        
        std::cout << "1. Gestionar Películas" << std::endl;
        std::cout << "2. Gestionar Salas" << std::endl;
        std::cout << "3. Gestionar Sesiones" << std::endl;
        std::cout << "4. Gestionar Usuarios" << std::endl;
        std::cout << "5. Ver Reportes" << std::endl;
        std::cout << "6. Cerrar sesión" << std::endl;
        
        int opcion = leerEntero("Seleccione una opción", 1, 6);
        
        switch (opcion) {
            case 1:
                mostrar(MENU_PELICULAS);
                break;
            case 2:
                mostrar(MENU_SALAS);
                break;
            case 3:
                mostrar(MENU_SESIONES);
                break;
            case 4:
                // Menú de usuarios (a implementar)
                break;
            case 5:
                mostrar(MENU_REPORTES);
                break;
            case 6:
                client->logout();
                mostrarExito("Sesión cerrada correctamente");
                pausar();
                mostrar(MENU_PRINCIPAL);
                return;
        }
    }
    
    // Si llegamos aquí, la sesión ha expirado o ya no es admin
    if (!client->isLoggedIn()) {
        mostrarError("La sesión ha expirado");
        pausar();
        mostrar(MENU_PRINCIPAL);
    }
}

void Menu::mostrarMenuCliente() {
    while (client->isLoggedIn()) {
        limpiarPantalla();
        mostrarEncabezado("MENÚ DE CLIENTE");
        
        std::cout << "1. Ver Cartelera" << std::endl;
        std::cout << "2. Comprar Entradas" << std::endl;
        std::cout << "3. Mis Compras" << std::endl;
        std::cout << "4. Cerrar sesión" << std::endl;
        
        int opcion = leerEntero("Seleccione una opción", 1, 4);
        
        switch (opcion) {
            case 1:
                clienteVerCartelera();
                break;
            case 2:
                clienteComprarEntradas();
                break;
            case 3:
                clienteVerCompras();
                break;
            case 4:
                client->logout();
                mostrarExito("Sesión cerrada correctamente");
                pausar();
                mostrar(MENU_PRINCIPAL);
                return;
        }
    }
    
    // Si llegamos aquí, la sesión ha expirado
    if (!client->isLoggedIn()) {
        mostrarError("La sesión ha expirado");
        pausar();
        mostrar(MENU_PRINCIPAL);
    }
}

void Menu::clienteVerCartelera() {
    limpiarPantalla();
    mostrarEncabezado("CARTELERA");
    
    // Obtener películas disponibles
    std::vector<Pelicula> peliculas = client->getPeliculas();
    
    if (peliculas.empty()) {
        std::cout << "No hay películas en cartelera actualmente." << std::endl;
        pausar();
        return;
    }
    
    std::cout << "PELÍCULAS EN CARTELERA:" << std::endl << std::endl;
    
    // Mostrar cada película con sus sesiones
    for (int i = 0; i < peliculas.size(); i++) {
        std::cout << std::endl << (i + 1) << ". " << peliculas[i].getTitulo() << std::endl;
        std::cout << "   Género: " << peliculas[i].getGenero() 
                  << " | Duración: " << peliculas[i].getDuracion() << " minutos" << std::endl;
        
        // Obtener sesiones para esta película
        std::vector<Sesion> sesiones = client->getSesionesByPelicula(peliculas[i].getId());
        
        if (!sesiones.empty()) {
            std::cout << "   Sesiones disponibles:" << std::endl;
            
            for (int j = 0; j < sesiones.size(); j++) {
                // Obtener sala
                Client::Sala sala = client->getSala(sesiones[j].getSalaId());
                
                // Contar asientos libres
                std::vector<Client::Asiento> asientos = client->getAsientosBySala(sala.id);
                int asientosLibres = 0;
                for (const auto& asiento : asientos) {
                    if (asiento.disponible) {
                        asientosLibres++;
                    }
                }
                
                // Formatear hora (quitar los segundos y la fecha completa)
                std::string horaInicio = sesiones[j].getHoraInicio().substr(11, 5);
                std::string horaFin = sesiones[j].getHoraFin().substr(11, 5);
                
                // Extraer solo la fecha (YYYY-MM-DD)
                std::string fecha = sesiones[j].getHoraInicio().substr(0, 10);
                
                std::cout << "     - Sesión ID: " << sesiones[j].getId() 
                          << " | Fecha: " << fecha 
                          << " | Hora: " << horaInicio << "-" << horaFin 
                          << " | Sala: " << sala.id 
                          << " | Asientos libres: " << asientosLibres << std::endl;
            }
        } else {
            std::cout << "   No hay sesiones disponibles para esta película." << std::endl;
        }
        
        std::cout << std::endl << "-------------------------------------------------" << std::endl;
    }
    
    pausar();
}

// Implementar el resto de las funciones del menú...

void Menu::mostrar(TipoMenu tipo) {
    switch (tipo) {
        case MENU_PRINCIPAL:
            mostrarMenuPrincipal();
            break;
        case MENU_AUTENTICACION:
            mostrarMenuAutenticacion();
            break;
        case MENU_ADMIN:
            mostrarMenuAdmin();
            break;
        case MENU_CLIENTE:
            mostrarMenuCliente();
            break;
        case MENU_PELICULAS:
            mostrarMenuPeliculas();
            break;
        case MENU_SALAS:
            mostrarMenuSalas();
            break;
        case MENU_SESIONES:
            mostrarMenuSesiones();
            break;
        case MENU_VENTAS:
            mostrarMenuVentas();
            break;
        case MENU_REPORTES:
            mostrarMenuReportes();
            break;
        default:
            mostrarError("Tipo de menú desconocido");
            break;
    }
}

void Menu::ejecutar() {
    active = true;
    while (active) {
        mostrar(MENU_PRINCIPAL);
    }
}