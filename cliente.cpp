#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <memory>
#include <cstring>

// Includes específicos para Windows
#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #define close closesocket
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
#endif

#define PORT 5000
#define MAX_BUFFER 4096

// ============================================================================
// CLIENTE DE RED CON RECONEXIÓN AUTOMÁTICA
// ============================================================================

class NetworkClient {
private:
    int sock;
    bool connected;
    
    bool reconectar() {
        if (connected) {
            close(sock);
            connected = false;
        }
        
        return conectar();
    }
    
public:
    NetworkClient() : sock(0), connected(false) {}
    
    ~NetworkClient() {
        if (connected) {
            desconectar();
        }
    }
    
    bool conectar() {
        struct sockaddr_in serv_addr;
        
        if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
            std::cout << "❌ Error creando socket" << std::endl;
            return false;
        }
        
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(PORT);
        
        #ifdef _WIN32
            serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
            if (serv_addr.sin_addr.s_addr == INADDR_NONE) {
                std::cout << "❌ Dirección inválida" << std::endl;
                return false;
            }
        #else
            if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
                std::cout << "❌ Dirección inválida" << std::endl;
                return false;
            }
        #endif
        
        if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
            std::cout << "❌ Error conectando al servidor" << std::endl;
            return false;
        }
        
        connected = true;
        return true;
    }
    
    std::string enviarComando(const std::string& comando) {
        // Intentar hasta 3 veces con reconexión
        for (int intento = 0; intento < 3; intento++) {
            if (!connected && !reconectar()) {
                return "ERROR:No se pudo conectar al servidor";
            }
            
            // Enviar comando
            if (send(sock, comando.c_str(), comando.length(), 0) < 0) {
                std::cout << "⚠️  Error enviando, reintentando..." << std::endl;
                connected = false;
                continue;
            }
            
            // Recibir respuesta
            char buffer[MAX_BUFFER] = {0};
            int bytes_received = recv(sock, buffer, MAX_BUFFER - 1, 0);
            
            if (bytes_received > 0) {
                buffer[bytes_received] = '\0';
                
                // El servidor desconecta después de cada comando
                close(sock);
                connected = false;
                
                return std::string(buffer);
            } else {
                std::cout << "⚠️  Sin respuesta del servidor, reintentando..." << std::endl;
                connected = false;
            }
        }
        
        return "ERROR:Sin respuesta del servidor después de varios intentos";
    }
    
    void desconectar() {
        if (connected) {
            close(sock);
            connected = false;
        }
    }
    
    bool estaConectado() const { return connected; }
};

// ============================================================================
// JERARQUÍA DE CLASES CON HERENCIA Y POLIMORFISMO
// ============================================================================

// Clase base abstracta Entity
class Entity {
protected:
    int id;
    
public:
    Entity(int id = 0) : id(id) {}
    virtual ~Entity() = default;
    
    // Métodos virtuales puros - polimorfismo
    virtual void mostrar() const = 0;
    virtual std::string getTipo() const = 0;
    
    // Getters y setters comunes
    int getId() const { return id; }
    void setId(int newId) { id = newId; }
};

// Clase base abstracta User (hereda de Entity)
class User : public Entity {
protected:
    std::string nombre;
    std::string correo;
    std::string telefono;
    
public:
    User(int id = 0, const std::string& nombre = "", const std::string& correo = "", 
         const std::string& telefono = "")
        : Entity(id), nombre(nombre), correo(correo), telefono(telefono) {}
    
    // Métodos virtuales puros - cada tipo de usuario los implementa diferente
    virtual void mostrarMenu() = 0;
    virtual std::vector<std::string> getPermisos() const = 0;
    virtual bool puedeGestionar(const std::string& recurso) const = 0;
    virtual std::string getRol() const = 0;
    
    // Método virtual que puede ser sobrescrito
    void mostrar() const override {
        std::cout << "Usuario: " << nombre << " (" << correo << ")" << std::endl;
    }
    
    std::string getTipo() const override { return "User"; }
    
    // Getters comunes
    const std::string& getNombre() const { return nombre; }
    const std::string& getCorreo() const { return correo; }
    const std::string& getTelefono() const { return telefono; }
};

// Declaraciones adelantadas de funciones de menú
void menuAdmin();
void menuCliente();
void menuGestionPeliculas();
void menuGestionSalas();
void menuGestionSesiones();
void mostrarCartelera();
void mostrarMisCompras();

// Clase AdminUser (hereda de User)
class AdminUser : public User {
public:
    AdminUser(int id = 0, const std::string& nombre = "", const std::string& correo = "", 
              const std::string& telefono = "")
        : User(id, nombre, correo, telefono) {}
    
    void mostrar() const override {
        std::cout << "👑 ADMINISTRADOR: " << nombre << " (" << correo << ")" << std::endl;
        std::cout << "   Permisos: Gestión completa del sistema" << std::endl;
    }
    
    std::string getTipo() const override { return "AdminUser"; }
    std::string getRol() const override { return "Administrador"; }
    
    std::vector<std::string> getPermisos() const override {
        return {
            "GESTIONAR_PELICULAS",
            "GESTIONAR_SALAS", 
            "GESTIONAR_SESIONES",
            "GESTIONAR_USUARIOS",
            "VER_REPORTES",
            "CONFIGURAR_SISTEMA"
        };
    }
    
    bool puedeGestionar(const std::string& recurso) const override {
        return true;
    }
    
    void mostrarMenu() override {
        menuAdmin();
    }
};

// Clase ClientUser (hereda de User)
class ClientUser : public User {
private:
    double saldo;
    
public:
    ClientUser(int id = 0, const std::string& nombre = "", const std::string& correo = "", 
               const std::string& telefono = "", double saldo = 0.0)
        : User(id, nombre, correo, telefono), saldo(saldo) {}
    
    void mostrar() const override {
        std::cout << "👤 CLIENTE: " << nombre << " (" << correo << ")" << std::endl;
        std::cout << "   Saldo disponible: " << saldo << "€" << std::endl;
    }
    
    std::string getTipo() const override { return "ClientUser"; }
    std::string getRol() const override { return "Cliente"; }
    
    std::vector<std::string> getPermisos() const override {
        return {
            "VER_CARTELERA",
            "COMPRAR_ENTRADAS",
            "VER_MIS_COMPRAS",
            "MODIFICAR_PERFIL"
        };
    }
    
    bool puedeGestionar(const std::string& recurso) const override {
        return (recurso == "PERFIL" || recurso == "MIS_COMPRAS");
    }
    
    void mostrarMenu() override {
        menuCliente();
    }
    
    bool tieneSaldoSuficiente(double precio) const {
        return saldo >= precio;
    }
    
    void restarSaldo(double cantidad) {
        if (cantidad <= saldo) {
            saldo -= cantidad;
        }
    }
    
    double getSaldo() const { return saldo; }
    void setSaldo(double nuevo_saldo) { saldo = nuevo_saldo; }
};

// Clase Movie
class Movie : public Entity {
private:
    std::string titulo;
    int duracion;
    std::string genero;
    
public:
    Movie(int id = 0, const std::string& titulo = "", int duracion = 0, const std::string& genero = "")
        : Entity(id), titulo(titulo), duracion(duracion), genero(genero) {}
    
    void mostrar() const override {
        std::cout << id << ". " << titulo << " (" << duracion << " min, " << genero << ")" << std::endl;
    }
    
    std::string getTipo() const override { return "Movie"; }
    
    const std::string& getTitulo() const { return titulo; }
    int getDuracion() const { return duracion; }
    const std::string& getGenero() const { return genero; }
    
    static std::vector<Movie> obtenerTodas();
    static bool crear(const std::string& titulo, int duracion, const std::string& genero);
};

class Room : public Entity {
private:
    int num_asientos;
    int asientos_libres;
    
public:
    Room(int id = 0, int asientos = 0, int libres = 0) 
        : Entity(id), num_asientos(asientos), asientos_libres(libres) {}
    
    void mostrar() const override {
        std::cout << "Sala " << id << " - " << num_asientos << " asientos ("
                  << asientos_libres << " libres)" << std::endl;
    }
    
    std::string getTipo() const override { return "Room"; }
    
    int getNumAsientos() const { return num_asientos; }
    int getAsientosLibres() const { return asientos_libres; }
    
    static std::vector<Room> obtenerTodas();
    static bool crear(int num_asientos);
};

class Session : public Entity {
private:
    int pelicula_id;
    int sala_id;
    std::string hora_inicio;
    std::string hora_fin;
    
public:
    Session(int id = 0, int pid = 0, int sid = 0, const std::string& inicio = "", const std::string& fin = "")
        : Entity(id), pelicula_id(pid), sala_id(sid), hora_inicio(inicio), hora_fin(fin) {}
    
    void mostrar() const override {
        std::cout << "Sesión " << id << " - Película: " << pelicula_id 
                  << ", Sala: " << sala_id << ", " << hora_inicio << " - " << hora_fin << std::endl;
    }
    
    std::string getTipo() const override { return "Session"; }
    
    int getPeliculaId() const { return pelicula_id; }
    int getSalaId() const { return sala_id; }
    const std::string& getHoraInicio() const { return hora_inicio; }
    const std::string& getHoraFin() const { return hora_fin; }
    
    static std::vector<Session> obtenerTodas();
    static std::vector<Session> obtenerPorPelicula(int movie_id);
};

class Sale : public Entity {
private:
    std::string fecha;
    double precio_total;
    
public:
    Sale(int id = 0, const std::string& fecha = "", double precio = 0.0)
        : Entity(id), fecha(fecha), precio_total(precio) {}
    
    void mostrar() const override {
        std::cout << "Compra " << id << " - " << fecha << " - " << precio_total << "€" << std::endl;
    }
    
    std::string getTipo() const override { return "Sale"; }
    
    const std::string& getFecha() const { return fecha; }
    double getPrecioTotal() const { return precio_total; }
    
    static std::vector<Sale> obtenerPorUsuario(int user_id);
};

// ============================================================================
// FACTORY PATTERN PARA CREAR USUARIOS
// ============================================================================

class UserFactory {
public:
    static std::unique_ptr<User> crearUsuario(const std::string& tipo, int id, 
                                            const std::string& nombre, 
                                            const std::string& correo,
                                            const std::string& telefono = "",
                                            double saldo = 0.0) {
        if (tipo == "admin" || tipo == "Administrador") {
            return std::make_unique<AdminUser>(id, nombre, correo, telefono);
        } else if (tipo == "cliente" || tipo == "Cliente") {
            return std::make_unique<ClientUser>(id, nombre, correo, telefono, saldo);
        }
        return nullptr;
    }
};

// ============================================================================
// SISTEMA DE AUTENTICACIÓN CON ESTADO PERSISTENTE
// ============================================================================

class AuthenticationService {
private:
    static std::unique_ptr<User> usuario_actual;
    static std::string email_actual;
    static std::string password_actual;
    
public:
    static bool login(const std::string& email, const std::string& password, NetworkClient& cliente) {
        std::string comando = "LOGIN:" + email + ":" + password;
        std::string respuesta = cliente.enviarComando(comando);
        
        std::cout << "🔍 Debug - Respuesta del servidor: " << respuesta << std::endl;
        
        if (respuesta.substr(0, 3) == "OK:") {
            // Parsear respuesta: "OK:id:nombre:tipo:correo:telefono:saldo"
            std::vector<std::string> datos = parsearRespuesta(respuesta);
            
            if (datos.size() >= 6) {
                int id = std::stoi(datos[1]);
                std::string nombre = datos[2];
                std::string tipo = datos[3];
                std::string correo = datos[4];
                std::string telefono = datos[5];
                double saldo = (datos.size() > 6) ? std::stod(datos[6]) : 0.0;
                
                // Guardar credenciales para reautenticación automática
                email_actual = email;
                password_actual = password;
                
                // Usar Factory para crear el usuario correcto
                usuario_actual = UserFactory::crearUsuario(tipo, id, nombre, correo, telefono, saldo);
                
                return usuario_actual != nullptr;
            }
        }
        
        return false;
    }
    
    static bool reautenticar(NetworkClient& cliente) {
        if (email_actual.empty()) return false;
        
        std::cout << "🔄 Reautenticando..." << std::endl;
        return login(email_actual, password_actual, cliente);
    }
    
    static void logout(NetworkClient& cliente) {
        if (usuario_actual) {
            cliente.enviarComando("LOGOUT");
            usuario_actual.reset();
            email_actual.clear();
            password_actual.clear();
        }
    }
    
    static User* getUsuarioActual() {
        return usuario_actual.get();
    }
    
    static bool isLoggedIn() {
        return usuario_actual != nullptr;
    }
    
    static bool isAdmin() {
        if (!usuario_actual) return false;
        return dynamic_cast<AdminUser*>(usuario_actual.get()) != nullptr;
    }
    
    static bool isClient() {
        if (!usuario_actual) return false;
        return dynamic_cast<ClientUser*>(usuario_actual.get()) != nullptr;
    }
    
private:
    static std::vector<std::string> parsearRespuesta(const std::string& respuesta) {
        std::vector<std::string> resultado;
        std::stringstream ss(respuesta);
        std::string item;
        
        while (std::getline(ss, item, ':')) {
            resultado.push_back(item);
        }
        
        return resultado;
    }
};

// Definición de las variables estáticas
std::unique_ptr<User> AuthenticationService::usuario_actual = nullptr;
std::string AuthenticationService::email_actual = "";
std::string AuthenticationService::password_actual = "";

// ============================================================================
// VARIABLES GLOBALES
// ============================================================================

NetworkClient cliente;

// ============================================================================
// IMPLEMENTACIONES DE MÉTODOS ESTÁTICOS CON REAUTENTICACIÓN
// ============================================================================

std::vector<Movie> Movie::obtenerTodas() {
    std::vector<Movie> peliculas;
    
    // Reautenticar antes de cada comando
    if (AuthenticationService::isLoggedIn()) {
        AuthenticationService::reautenticar(cliente);
    }
    
    std::string respuesta = cliente.enviarComando("GET_MOVIES");
    std::cout << "🔍 Debug GET_MOVIES - Respuesta: " << respuesta << std::endl;
    
    if (respuesta.substr(0, 3) == "OK:") {
        std::string datos = respuesta.substr(3);
        std::stringstream ss(datos);
        std::string pelicula_str;
        
        while (std::getline(ss, pelicula_str, ';')) {
            if (pelicula_str.empty()) continue;
            
            std::stringstream ps(pelicula_str);
            std::string campo;
            std::vector<std::string> campos;
            
            while (std::getline(ps, campo, '|')) {
                campos.push_back(campo);
            }
            
            if (campos.size() >= 4) {
                int id = std::stoi(campos[0]);
                std::string titulo = campos[1];
                int duracion = std::stoi(campos[2]);
                std::string genero = campos[3];
                
                peliculas.emplace_back(id, titulo, duracion, genero);
            }
        }
    }
    
    return peliculas;
}

bool Movie::crear(const std::string& titulo, int duracion, const std::string& genero) {
    // Reautenticar antes de crear
    if (AuthenticationService::isLoggedIn()) {
        AuthenticationService::reautenticar(cliente);
    }
    
    std::string comando = "CREATE_MOVIE:" + titulo + ":" + std::to_string(duracion) + ":" + genero;
    std::string respuesta = cliente.enviarComando(comando);
    std::cout << "🔍 Debug CREATE_MOVIE - Respuesta: " << respuesta << std::endl;
    
    return respuesta.substr(0, 2) == "OK";
}

std::vector<Room> Room::obtenerTodas() {
    std::vector<Room> salas;
    
    if (AuthenticationService::isLoggedIn()) {
        AuthenticationService::reautenticar(cliente);
    }
    
    std::string respuesta = cliente.enviarComando("GET_ROOMS");
    
    if (respuesta.substr(0, 3) == "OK:") {
        std::string datos = respuesta.substr(3);
        std::stringstream ss(datos);
        std::string sala_str;
        
        while (std::getline(ss, sala_str, ';')) {
            if (sala_str.empty()) continue;
            
            std::stringstream ps(sala_str);
            std::string campo;
            std::vector<std::string> campos;
            
            while (std::getline(ps, campo, '|')) {
                campos.push_back(campo);
            }
            
            if (campos.size() >= 3) {
                int id = std::stoi(campos[0]);
                int asientos = std::stoi(campos[1]);
                int libres = std::stoi(campos[2]);
                
                salas.emplace_back(id, asientos, libres);
            }
        }
    }
    
    return salas;
}

bool Room::crear(int num_asientos) {
    if (AuthenticationService::isLoggedIn()) {
        AuthenticationService::reautenticar(cliente);
    }
    
    std::string comando = "CREATE_ROOM:" + std::to_string(num_asientos);
    std::string respuesta = cliente.enviarComando(comando);
    return respuesta.substr(0, 2) == "OK";
}

std::vector<Session> Session::obtenerTodas() {
    std::vector<Session> sesiones;
    
    if (AuthenticationService::isLoggedIn()) {
        AuthenticationService::reautenticar(cliente);
    }
    
    std::string respuesta = cliente.enviarComando("GET_SESSIONS");
    
    if (respuesta.substr(0, 3) == "OK:") {
        std::string datos = respuesta.substr(3);
        std::stringstream ss(datos);
        std::string sesion_str;
        
        while (std::getline(ss, sesion_str, ';')) {
            if (sesion_str.empty()) continue;
            
            std::stringstream ps(sesion_str);
            std::string campo;
            std::vector<std::string> campos;
            
            while (std::getline(ps, campo, '|')) {
                campos.push_back(campo);
            }
            
            if (campos.size() >= 5) {
                int id = std::stoi(campos[0]);
                int pid = std::stoi(campos[1]);
                int sid = std::stoi(campos[2]);
                std::string inicio = campos[3];
                std::string fin = campos[4];
                
                sesiones.emplace_back(id, pid, sid, inicio, fin);
            }
        }
    }
    
    return sesiones;
}

std::vector<Session> Session::obtenerPorPelicula(int movie_id) {
    std::vector<Session> sesiones;
    
    if (AuthenticationService::isLoggedIn()) {
        AuthenticationService::reautenticar(cliente);
    }
    
    std::string comando = "GET_SESSIONS_BY_MOVIE:" + std::to_string(movie_id);
    std::string respuesta = cliente.enviarComando(comando);
    
    if (respuesta.substr(0, 3) == "OK:") {
        std::string datos = respuesta.substr(3);
        std::stringstream ss(datos);
        std::string sesion_str;
        
        while (std::getline(ss, sesion_str, ';')) {
            if (sesion_str.empty()) continue;
            
            std::stringstream ps(sesion_str);
            std::string campo;
            std::vector<std::string> campos;
            
            while (std::getline(ps, campo, '|')) {
                campos.push_back(campo);
            }
            
            if (campos.size() >= 5) {
                int id = std::stoi(campos[0]);
                int pid = std::stoi(campos[1]);
                int sid = std::stoi(campos[2]);
                std::string inicio = campos[3];
                std::string fin = campos[4];
                
                sesiones.emplace_back(id, pid, sid, inicio, fin);
            }
        }
    }
    
    return sesiones;
}

std::vector<Sale> Sale::obtenerPorUsuario(int user_id) {
    std::vector<Sale> ventas;
    
    if (AuthenticationService::isLoggedIn()) {
        AuthenticationService::reautenticar(cliente);
    }
    
    std::string comando = "GET_USER_PURCHASES:" + std::to_string(user_id);
    std::string respuesta = cliente.enviarComando(comando);
    
    if (respuesta.substr(0, 3) == "OK:") {
        std::string datos = respuesta.substr(3);
        std::stringstream ss(datos);
        std::string venta_str;
        
        while (std::getline(ss, venta_str, ';')) {
            if (venta_str.empty()) continue;
            
            std::stringstream ps(venta_str);
            std::string campo;
            std::vector<std::string> campos;
            
            while (std::getline(ps, campo, '|')) {
                campos.push_back(campo);
            }
            
            if (campos.size() >= 3) {
                int id = std::stoi(campos[0]);
                std::string fecha = campos[1];
                double precio = std::stod(campos[2]);
                
                ventas.emplace_back(id, fecha, precio);
            }
        }
    }
    
    return ventas;
}

// ============================================================================
// FUNCIONES DE UTILIDAD
// ============================================================================

void limpiarPantalla() {
    system("cls");
}

void pausar() {
    std::cout << "\nPresione Enter para continuar...";
    std::cin.ignore();
    std::cin.get();
}

int leerEntero(const std::string& prompt, int min, int max) {
    int valor;
    while (true) {
        std::cout << prompt << " (" << min << "-" << max << "): ";
        std::cin >> valor;
        
        if (std::cin.fail() || valor < min || valor > max) {
            std::cin.clear();
            std::cin.ignore(10000, '\n');
            std::cout << "❌ Valor inválido. Intente de nuevo." << std::endl;
        } else {
            std::cin.ignore();
            return valor;
        }
    }
}

std::string leerTexto(const std::string& prompt) {
    std::string texto;
    std::cout << prompt << ": ";
    std::getline(std::cin, texto);
    return texto;
}

bool confirmar(const std::string& prompt) {
    std::string respuesta;
    std::cout << prompt << " (s/n): ";
    std::getline(std::cin, respuesta);
    return (respuesta == "s" || respuesta == "S" || respuesta == "si" || respuesta == "SI");
}

void mostrarError(const std::string& mensaje) {
    std::cout << "❌ Error: " << mensaje << std::endl;
}

void mostrarExito(const std::string& mensaje) {
    std::cout << "✅ " << mensaje << std::endl;
}

// ============================================================================
// FUNCIONES DE MENÚ
// ============================================================================

void mostrarMenuPrincipal() {
    limpiarPantalla();
    std::cout << "========================================" << std::endl;
    std::cout << "    🎬 SISTEMA DE GESTIÓN DE CINE 🎬" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "1. Iniciar sesión" << std::endl;
    std::cout << "2. Salir" << std::endl;
    std::cout << "========================================" << std::endl;
}

bool login() {
    limpiarPantalla();
    std::cout << "=== 🔐 INICIAR SESIÓN ===" << std::endl;
    
    std::string email = leerTexto("Correo electrónico");
    std::string password = leerTexto("Contraseña");
    
    if (AuthenticationService::login(email, password, cliente)) {
        std::cout << "\n🎉 ¡Bienvenido!" << std::endl;
        AuthenticationService::getUsuarioActual()->mostrar();
        pausar();
        return true;
    } else {
        mostrarError("Credenciales incorrectas");
        pausar();
        return false;
    }
}

void logout() {
    AuthenticationService::logout(cliente);
    mostrarExito("Sesión cerrada correctamente");
    pausar();
}

// ============================================================================
// MENÚS ESPECÍFICOS PARA ADMINISTRADOR
// ============================================================================

void menuGestionPeliculas() {
    while (true) {
        limpiarPantalla();
        std::cout << "=== 🎬 GESTIÓN DE PELÍCULAS ===" << std::endl;
        std::cout << "1. Ver todas las películas" << std::endl;
        std::cout << "2. Añadir película" << std::endl;
        std::cout << "3. Volver al menú anterior" << std::endl;
        
        int opcion = leerEntero("Seleccione una opción", 1, 3);
        
        switch (opcion) {
            case 1: {
                limpiarPantalla();
                std::cout << "=== 📋 LISTA DE PELÍCULAS ===" << std::endl;
                
                auto peliculas = Movie::obtenerTodas();
                if (peliculas.empty()) {
                    std::cout << "📭 No hay películas registradas." << std::endl;
                } else {
                    for (const auto& pelicula : peliculas) {
                        pelicula.mostrar();
                    }
                }
                pausar();
                break;
            }
            case 2: {
                limpiarPantalla();
                std::cout << "=== ➕ AÑADIR PELÍCULA ===" << std::endl;
                
                std::string titulo = leerTexto("Título de la película");
                int duracion = leerEntero("Duración en minutos", 1, 500);
                std::string genero = leerTexto("Género");
                
                if (Movie::crear(titulo, duracion, genero)) {
                    mostrarExito("Película creada exitosamente");
                } else {
                    mostrarError("No se pudo crear la película");
                }
                pausar();
                break;
            }
            case 3:
                return;
        }
    }
}

void menuGestionSalas() {
    while (true) {
        limpiarPantalla();
        std::cout << "=== 🏢 GESTIÓN DE SALAS ===" << std::endl;
        std::cout << "1. Ver todas las salas" << std::endl;
        std::cout << "2. Añadir sala" << std::endl;
        std::cout << "3. Volver al menú anterior" << std::endl;
        
        int opcion = leerEntero("Seleccione una opción", 1, 3);
        
        switch (opcion) {
            case 1: {
                limpiarPantalla();
                std::cout << "=== 📋 LISTA DE SALAS ===" << std::endl;
                
                auto salas = Room::obtenerTodas();
                if (salas.empty()) {
                    std::cout << "📭 No hay salas registradas." << std::endl;
                } else {
                    for (const auto& sala : salas) {
                        sala.mostrar();
                    }
                }
                pausar();
                break;
            }
            case 2: {
                limpiarPantalla();
                std::cout << "=== ➕ AÑADIR SALA ===" << std::endl;
                
                int num_asientos = leerEntero("Número de asientos", 1, 1000);
                
                if (Room::crear(num_asientos)) {
                    mostrarExito("Sala creada exitosamente");
                } else {
                    mostrarError("No se pudo crear la sala");
                }
                pausar();
                break;
            }
            case 3:
                return;
        }
    }
}

void menuGestionSesiones() {
    while (true) {
        limpiarPantalla();
        std::cout << "=== 🎭 GESTIÓN DE SESIONES ===" << std::endl;
        std::cout << "1. Ver todas las sesiones" << std::endl;
        std::cout << "2. Ver sesiones por película" << std::endl;
        std::cout << "3. Volver al menú anterior" << std::endl;
        
        int opcion = leerEntero("Seleccione una opción", 1, 3);
        
        switch (opcion) {
            case 1: {
                limpiarPantalla();
                std::cout << "=== 📋 LISTA DE SESIONES ===" << std::endl;
                
                auto sesiones = Session::obtenerTodas();
                if (sesiones.empty()) {
                    std::cout << "📭 No hay sesiones registradas." << std::endl;
                } else {
                    for (const auto& sesion : sesiones) {
                        sesion.mostrar();
                    }
                }
                pausar();
                break;
            }
            case 2: {
                limpiarPantalla();
                std::cout << "=== 🔍 SESIONES POR PELÍCULA ===" << std::endl;
                
                auto peliculas = Movie::obtenerTodas();
                if (peliculas.empty()) {
                    std::cout << "📭 No hay películas disponibles." << std::endl;
                    pausar();
                    break;
                }
                
                std::cout << "Películas disponibles:" << std::endl;
                for (const auto& pelicula : peliculas) {
                    pelicula.mostrar();
                }
                
                int movie_id = leerEntero("ID de la película", 1, 9999);
                
                auto sesiones = Session::obtenerPorPelicula(movie_id);
                if (sesiones.empty()) {
                    std::cout << "\n📭 No hay sesiones para esta película." << std::endl;
                } else {
                    std::cout << "\n✅ Sesiones encontradas:" << std::endl;
                    for (const auto& sesion : sesiones) {
                        sesion.mostrar();
                    }
                }
                pausar();
                break;
            }
            case 3:
                return;
        }
    }
}

void menuAdmin() {
    while (true) {
        limpiarPantalla();
        std::cout << "=== 👑 MENÚ ADMINISTRADOR ===" << std::endl;
        AuthenticationService::getUsuarioActual()->mostrar();
        std::cout << "\n1. Gestionar Películas" << std::endl;
        std::cout << "2. Gestionar Salas" << std::endl;
        std::cout << "3. Gestionar Sesiones" << std::endl;
        std::cout << "4. Cerrar sesión" << std::endl;
        
        int opcion = leerEntero("Seleccione una opción", 1, 4);
        
        switch (opcion) {
            case 1:
                menuGestionPeliculas();
                break;
            case 2:
                menuGestionSalas();
                break;
            case 3:
                menuGestionSesiones();
                break;
            case 4:
                logout();
                return;
        }
    }
}

// ============================================================================
// MENÚS ESPECÍFICOS PARA CLIENTE
// ============================================================================

void mostrarCartelera() {
    limpiarPantalla();
    std::cout << "=== 🎬 CARTELERA ===" << std::endl;
    
    auto peliculas = Movie::obtenerTodas();
    if (peliculas.empty()) {
        std::cout << "📭 No hay películas en cartelera." << std::endl;
    } else {
        for (const auto& pelicula : peliculas) {
            pelicula.mostrar();
            
            auto sesiones = Session::obtenerPorPelicula(pelicula.getId());
            if (!sesiones.empty()) {
                std::cout << "  🎭 Sesiones disponibles:" << std::endl;
                for (const auto& sesion : sesiones) {
                    std::cout << "    ";
                    sesion.mostrar();
                }
            } else {
                std::cout << "  ❌ No hay sesiones disponibles." << std::endl;
            }
            std::cout << std::endl;
        }
    }
    pausar();
}

void mostrarMisCompras() {
    limpiarPantalla();
    std::cout << "=== 🛒 MIS COMPRAS ===" << std::endl;
    
    User* usuario = AuthenticationService::getUsuarioActual();
    auto compras = Sale::obtenerPorUsuario(usuario->getId());
    
    if (compras.empty()) {
        std::cout << "📭 No has realizado ninguna compra." << std::endl;
    } else {
        for (const auto& compra : compras) {
            compra.mostrar();
        }
    }
    pausar();
}

void menuCliente() {
    while (true) {
        limpiarPantalla();
        std::cout << "=== 👤 MENÚ CLIENTE ===" << std::endl;
        AuthenticationService::getUsuarioActual()->mostrar();
        std::cout << "\n1. Ver Cartelera" << std::endl;
        std::cout << "2. Mis Compras" << std::endl;
        std::cout << "3. Cerrar sesión" << std::endl;
        
        int opcion = leerEntero("Seleccione una opción", 1, 3);
        
        switch (opcion) {
            case 1:
                mostrarCartelera();
                break;
            case 2:
                mostrarMisCompras();
                break;
            case 3:
                logout();
                return;
        }
    }
}

// ============================================================================
// FUNCIÓN PRINCIPAL
// ============================================================================

int main() {
    #ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2,2), &wsaData) != 0) {
        std::cout << "❌ Error inicializando Winsock" << std::endl;
        return -1;
    }
    #endif
    
    std::cout << "🎬 === CLIENTE SISTEMA DE CINE === 🎬" << std::endl;
    std::cout << "Preparando conexión al servidor..." << std::endl;
    
    std::cout << "✅ ¡Sistema listo para conectar!" << std::endl;
    pausar();
    
    while (true) {
        mostrarMenuPrincipal();
        
        int opcion = leerEntero("Seleccione una opción", 1, 2);
        
        switch (opcion) {
            case 1:
                if (login()) {
                    User* usuario = AuthenticationService::getUsuarioActual();
                    usuario->mostrarMenu();
                }
                break;
                
            case 2:
                cliente.enviarComando("QUIT");
                cliente.desconectar();
                std::cout << "\n👋 ¡Hasta luego!" << std::endl;
                #ifdef _WIN32
                WSACleanup();
                #endif
                return 0;
        }
    }
    
    #ifdef _WIN32
    WSACleanup();
    #endif
    return 0;
}