// menu.h
#ifndef MENU_CPP_H
#define MENU_CPP_H

#include "client.h"
#include <string>
#include <functional>

enum TipoMenu {
    MENU_PRINCIPAL,
    MENU_AUTENTICACION,
    MENU_ADMIN,
    MENU_CLIENTE,
    MENU_PELICULAS,
    MENU_SALAS,
    MENU_SESIONES,
    MENU_VENTAS,
    MENU_REPORTES
};

class Menu {
private:
    Client* client;
    bool active;
    
    // Funciones de utilidad para la interfaz
    void limpiarPantalla();
    void mostrarEncabezado(const std::string& titulo);
    void mostrarPiePagina();
    void mostrarError(const std::string& mensaje);
    void mostrarExito(const std::string& mensaje);
    void pausar();
    
    std::string leerTexto(const std::string& prompt);
    int leerEntero(const std::string& prompt, int min, int max);
    double leerDecimal(const std::string& prompt, double min, double max);
    bool confirmar(const std::string& prompt);
    
    // Menús específicos
    void mostrarMenuPrincipal();
    void mostrarMenuAutenticacion();
    void mostrarMenuAdmin();
    void mostrarMenuCliente();
    void mostrarMenuPeliculas();
    void mostrarMenuSalas();
    void mostrarMenuSesiones();
    void mostrarMenuVentas();
    void mostrarMenuReportes();
    
    // Funciones del cliente
    void clienteVerCartelera();
    void clienteComprarEntradas();
    void clienteVerCompras();
    void clienteMostrarAsientos(int sesionId);
    void clienteMostrarDetalleVenta(int ventaId);

public:
    Menu(Client* client);
    ~Menu();
    
    void mostrar(TipoMenu tipo);
    void ejecutar();
};

#endif // MENU_CPP_H